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
broadcast_socket::broadcast_socket(int destPort, int inPort)
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
    //int inPort = port;
    
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
    localAddr.sin_port = htons(inPort);

    if (bind(sock_fd, (sockaddr*)&localAddr, sizeof(struct sockaddr_in)) == -1)
    {
        //TODO - Throw exception
        std::cout << "bind failed miserably" << std::endl;
        exit(-1);
    }

    std::cout << "Socket bound" << std::endl;

    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);
    broadcastAddr.sin_port = htons(destPort);
    
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

            //Calculate CRC32
            uint32_t msgCRC = stagedMsg.getCRC32();
            stagedMsg.CRC = msgCRC;

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
        //Timing is critical, so we take note of the time right away
        recvTime = std::chrono::high_resolution_clock::now();

        //Reject messages that do not have the appropriate size
        if (receivedBytes != TCAS_MSG_LEN)
        {
            continue;
        }
        TCAS_msg *msgBuffer = (TCAS_msg *)recvBuffer;
        tempRemoteMsg = *msgBuffer; //Convenience

        //Ignore our own messages and invalid hex
        if (tempRemoteMsg.ac_id != stagedMsg.ac_id && tempRemoteMsg.ac_id != 0)
        {
            std::cout << "Received " << receivedBytes << " bytes!" << std::endl;   
            std::string output = tempRemoteMsg.toString();
            std::cout << "Begin TCAS message:" << std::endl;
            std::cout << output;
            std::cout << "End TCAS message." << std::endl;

            //TODO: VALIDATE CHECKSUM

            if (!processTarget(tempRemoteMsg,recvTime))
            {
                //Log an error
                std::cerr << "WARNING: Too many targets! New target dropped.";
                std::cerr << std::endl;
            }

            //TODO: Check if the message is old
        }
    }
}  



/*
 *  processTarget: takes a TCAS_msg and uses it to update
 *  the relevant data in the targets list.
    Converts received messages to our internal format and adds
    them to the list.
 *  AC_state is initialized with the given time_point.
 *
 *  Returns true on success, false if there is no room for 
 *  a new target.
 */
bool broadcast_socket::processTarget(TCAS_msg tgtMsg, 
        std::chrono::high_resolution_clock::time_point recvTime)
//AC_state newState, TCAS_state newTCAS
{ 
    AC_state newState = AC_state(tgtMsg, recvTime);
    TCAS_state newTCAS = TCAS_state(tgtMsg);
    
    //Lock the targets list
    std::lock_guard<std::mutex> lock(targetsMutex);
    //No need to unlock, destroying the object releases the lock
    
    //find target somewhere
    for (int i = 0; i < MAX_TARGETS; i++)
    {
        if (newState.AC_ID == targetsList[i].AC_ID)
        {
            targetsList[i] = newState;
            targetsTCAS[i] = newTCAS;
            return true;
        }
    }
    
    //Aircraft not in targets. Insert in first available.
    for (int i = 0; i < MAX_TARGETS; i++)
    {
        if (targetsList[i].AC_ID == 0)
        {
            targetsList[i] = newState;
            targetsTCAS[i] = newTCAS;
            return true;
        }
    }
    //No room for new ones. Welp.
    return false;
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
    //Lock our position data
    std::lock_guard<std::mutex> lock(ownStateMutex);
    //No need to unlock, destroying the object releases the lock
    
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
    //Lock our position data
    std::lock_guard<std::mutex> lock(ownStateMutex);
    //No need to unlock, destroying the object releases the lock
    
    stagedMsg.updateOwnStatus(ownState);
    msgUpdated = true;
}  



/*
 *  getUpdatedTargetsStatus is called from an external thread.
 *
 *  It fills in the vectors referenced in the arguments with
 *  the data from the current list.
 *
 *  The return value represents the number of targets in the list
 */
int broadcast_socket::getUpdatedTargetsStatus(
            std::vector<AC_state>& targetsStatusVector,
            std::vector<TCAS_state>& targetsTCASVector)
{
    //Preallocate for time efficiency
    targetsStatusVector.reserve(MAX_TARGETS);
    targetsTCASVector.reserve(MAX_TARGETS);

    //Ensure the vectors are empty
    targetsStatusVector.clear();
    targetsTCASVector.clear();
    
    //Lock the targets list
    std::lock_guard<std::mutex> lock(targetsMutex);
    //No need to unlock, destroying the object releases the lock

    int targetsTracked = 0;
    for (int i = 0; i < MAX_TARGETS; i++)
    {
        //Check for valid ID
        if (targetsList[i].AC_ID != 0)
        {
            targetsTracked++;
            targetsStatusVector.push_back(targetsList[i]);
            targetsTCASVector.push_back(targetsTCAS[i]);
        }
    }

    return targetsTracked;
}



    
    
    
    
    
    
    
    
