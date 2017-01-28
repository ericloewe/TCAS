//TCAS_comms.cpp

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "TCAS_comms.h"



 
/*
 *  Init constructor
 *
 *  Its tasks are as follows:
 *      - Open the broadcast socket (using the given port)
 *      - Spawn send and recv threads
 */ 
broadcast_socket::broadcast_socket(int port)
{
    //Create two IPv4 UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sock_fd < 1)
    {
        //TODO - Throw exception
        std::cout << "Socket creation failed and should've thrown an exception" << std::endl;
        exit(-1);
    }
    
    char broadcastIP[] = "255.255.255.255";
    int broadcastPort = port;
    
    int broadcastPerm = 1;
    
    //Set broadcast permission to true
    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, 
                   (void *) &broadcastPerm, sizeof(broadcastPerm)) < 0)
    {
        //TODO - Throw exception
        std::cout << "setsockopt failed miserably" << std::endl;
        exit(-1);
    }
    
    //Set loopback to false
    
    char loopch = 0;

    if (setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_LOOP,
                    (char *)&loopch, sizeof(loopch)) < 0) 
    {
        perror("setting IP_MULTICAST_LOOP:");
        close(sock_fd);
        exit(1);
    }
    
    

    //Bind the socket 
    struct sockaddr_in localAddr;
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_BROADCAST;
    localAddr.sin_port = htons(broadcastPort);

    if (bind(sock_fd, (sockaddr*)&localAddr, sizeof(struct sockaddr_in)) == -1)
    {
        //TODO - Throw exception
        std::cout << "bind failed miserably" << std::endl;
        exit(-1);
    }

    std::cout << "Socket bound" << std::endl;

    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);
    broadcastAddr.sin_port = htons(broadcastPort);
    
    //Paperwork to expedite future sends
    sendAddr = (sockaddr*) &broadcastAddr;
    sendAddrSize = sizeof(broadcastAddr);

    //Doesn't work, returns 255.255.255.255
    /*//Determine our own address so we can reject it later
    unsigned int ownAddrLen = sizeof(struct sockaddr_in);
    if (getsockname(sock_fd, (sockaddr*)&ownAddr, &ownAddrLen))
    {
        if (ownAddrLen != sizeof(struct sockaddr_in))
        {
            //TODO - throw an exception
            std::cout << "getsockopt: address is not the same ";
            std::cout << "size as struct sockaddr_in!" << std::endl;
        }
    }*/

    // Spawn the helper threads
    sendThread = std::thread(&broadcast_socket::sendThreadFunction, this);
    recvThread = std::thread(&broadcast_socket::recvThreadFunction, this);

}



/* Destructor

   Close socket?

   Joins the spawned threads for neatness
*/
broadcast_socket::~broadcast_socket()
{
    //This cleanup is necessary to prevent a core dump
    // TODO: Threads must receive a signal to stop their execution
    sendThread.join();
    recvThread.join();

    std::cout << "Joined all broadcast socket threads" << std::endl;
}



/*
 * This is the function run in sendThread.
 *
 *  Its tasks are:
 *  - Set up the timer for 1s
 *  - Send the updated own status, if available
 *  - Increment other A/C's counters
 *  - Remove aircraft that have been inactive lfor a while
 *  - Wait for the timer
 */
void broadcast_socket::sendThreadFunction()
{
    std::cout << "sendThread was started!" << std::endl;
    std::cout << "sendThread setting up timer" << std::endl;

    nextSend = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));
    
    while (continueSend)
    {
        //Set the time when we next want to wake up (1s)
        //AFAICT, this stuff is all in nanoseconds
        nextSend += oneSecond;
        std::cout << "sendThread: tick - next send at: ";
        auto nextSendTime = nextSend.time_since_epoch();
        std::cout << nextSendTime.count() << std::endl;

        if (!msgInitialized)
        {
            //No point logging errors, we're not there yet
            std::this_thread::sleep_until(nextSend);
            continue;
        }

        if (msgUpdated)
        {
            //Lock our position data
            std::lock_guard<std::mutex> lock(ownStateMutex);
            //No need to unlock, destroying the object releases the lock

            msgUpdated = false;

            if (sendto(sock_fd, (void*)&stagedMsg, TCAS_MSG_LEN, 0, 
                sendAddr, sendAddrSize) != TCAS_MSG_LEN)
            {
                //TODO throw exception
                fprintf(stderr, "sendto error");
                exit(1);
            }
        }
        else //Log an error, the other end needs to keep up
        {
            std::cout << "sendThread: WARNING - Tick, but no update." << std::endl;
        }

        std::this_thread::sleep_until(nextSend);
    }
}



/*
 *  This is the function run in recvThread.
 *
 *      It continuously runs recv() and processes the messages
 */
void broadcast_socket::recvThreadFunction()
{
    int receivedBytes;
    std::cout << "recvThread was initiated!" << std::endl;

    char recvBuffer[TCAS_MSG_LEN];

    struct sockaddr_in  recvAddr;
    unsigned int        recvAddrSize;

    while (!msgInitialized)
    {
        //Avoid a race so that we only start receiving once our
        //ID is set
        sleep(1);
    }

    while (continueRecv)
    {
        recvAddrSize = sizeof(struct sockaddr_in);

        receivedBytes = recvfrom(sock_fd, (void *)recvBuffer, TCAS_MSG_LEN,
                                   0, (sockaddr*)&recvAddr, &recvAddrSize);

        //Reject messages that do not have the appropriate size
        if (receivedBytes != TCAS_MSG_LEN)
        {
            continue;
        }
        TCAS_msg *msgBuffer = (TCAS_msg *)recvBuffer;

        //Ignore our own messages
        if (msgBuffer -> ac_id != OWN_HEX)
        {
            std::cout << "Received " << receivedBytes << " bytes!" << std::endl;
            
            tempRemoteMsg = *msgBuffer;
            std::string output = tempRemoteMsg.toString();
            std::cout << "Begin TCAS message:" << std::endl;
            std::cout << output;
            std::cout << "End TCAS message." << std::endl;

            //TODO:
            //Validate Checksum
            //Add message if new
            //Etc.
        }
    }
}  



/*
 *  Initialize TCAS data. Just a formality so that we can start
 *  transmitting.
 *  Heavy lifting is done by updateStatus();
 *
 */
void broadcast_socket::initializeStatus(AC_state ownState, TCAS_state tcasSituation)
{
    updateStatus(ownState, tcasSituation);
    //Add the header
    strncpy(stagedMsg.header, TCAS_MSG_HEADER, TCAS_MSG_STRLEN);
    msgInitialized = true;
}



/*
 *  Update all TCAS data
 *
 *  Used when a conflict was detected or similar situations
 *
 */
void broadcast_socket::updateStatus(AC_state ownState, TCAS_state tcasSituation)
{
    stagedMsg.updateOwnStatus(ownState);
    stagedMsg.updateTCASStatus(tcasSituation);
    msgUpdated = true;
}   



/*
 *  Update just our vectors
 *
 *  Used when nothing much happens
 *
 */
void broadcast_socket::updateStatus(AC_state ownState)
{
    stagedMsg.updateOwnStatus(ownState);
    msgUpdated = true;
}  



/*
 *  updateTCASStatus - Update the message's own state fields
 */
void TCAS_msg::updateOwnStatus(AC_state state)
{
    ac_id = state.getID();
    
    xPos = state.getX_pos();
    yPos = state.getY_pos();
    zPos = state.getZ_pos();
    
    xSpd = state.getX_spd();
    ySpd = state.getY_spd();
    zSpd = state.getZ_spd();
}



/*
 *  updateTCASStatus - Update the message's TCAS fields
 */
void TCAS_msg::updateTCASStatus(TCAS_state state)
{
    strncpy(status, state.status, 16);
    strncpy(resolution, state.resolution, 16);
    intruderHex = state.intruder_hex;
    resValue = state.res_value;
}



/*  
 *  Constructor: Nothing is known.
 */
TCAS_msg::TCAS_msg()
{
    //Nothing to construct.
}



/*  
 *  Constructor: Use existing AC_state to initialize the message.
 *
 *      TCAS status data is not initialized
 *
 *      CRC32 is calculated only upon transmission
 */
TCAS_msg::TCAS_msg(AC_state state)
{
    //Set the correct header
    strncpy(header, TCAS_MSG_HEADER, TCAS_MSG_STRLEN);
    
    updateOwnStatus(state);
    
    //We don't have TCAS status data
    
    //CRC32 is calculated just before transmission
}



/*  
 *  Constructor: Use existing AC_state and TCAS_state to initialize 
 *                  the message.
 *
 *      CRC32 is calculated only upon transmission
 */
TCAS_msg::TCAS_msg(AC_state state, TCAS_state situation)
{
    //Set the correct header
    strncpy(header, TCAS_MSG_HEADER, TCAS_MSG_STRLEN);
    
    updateOwnStatus(state);
    updateTCASStatus(situation);
    
    //CRC32 is calculated just before transmission
}



/*  
 *  Due to compiler optimizations, sizeof(TCAS_msg) is
 *  128 bytes. The real message size is only 124 bytes.
 *  We can't just send four extra bytes willy-nilly.
 *  Well, we *could*, but other people would not be happy
 *  as we'd probably expose weird edge cases in their code...
 *
 *  Returns 124. Yes, that's it.
 */    
int TCAS_msg::nonPaddedSize()
{
    return TCAS_MSG_LEN;
}



/*
 *  TCAS_msg::toString()
 *
 *  Returns a pretty-format string to display
 *  a TCAS message to the user
 *
 */
 std::string TCAS_msg::toString()
 {
     std::stringstream output;
     
     output << header;
     output << std::endl << "Aircraft ID: ";
     output << ac_id << std::endl;
     output << "Position: ";
     output << (xPos) << "  ";
     output << (yPos) << "  ";
     output << (zPos) << std::endl;
     output << "Velocity: ";
     output << xSpd << "  ";
     output << ySpd << "  ";
     output << zSpd << std::endl;
     output << "Conflict info:" << std::endl;
     output << status << std::endl;
     output << "Intruder ID: ";
     output << intruderHex << std::endl;
     output << "Resolution: ";
     output << resolution << std::endl;
     output << "Res. Value: ";
     output << resValue << std::endl;

     return output.str();
 }

    
    
    
    
    
    
    
    
