//  TCAS_comms.h

#include <exception> 
#include <thread>
#include <mutex>
#include <new>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "TCAS_defs.h"

#pragma once

#define DEFAULT_MSG_INIT_SIZE 256 //???
#define TCAS_MSG_LEN 124

#define TCAS_MSG_HEADER "ACIP TCAS   V01\0"
#define TCAS_MSG_STRLEN 16




/*  
 *  TCAS_msg is structured exactly like the data sent over the network. 
 *  Instances of this class can simply be cast into void * and passed 
 *  to/from socket's send and recv functions.
 *
 *  Or rather, this would be possible without compiler optimizations.
 *  Most compilers will pad 124 bytes to 128 bytes, so care still needs 
 *  to be taken.
 *  
 */ 
class TCAS_msg
{
//private:
    

public:
    
    char        header[16];
    uint64_t    ac_id;
    
    double      xPos;
    double      yPos;
    double      zPos;
    
    double      xSpd;
    double      ySpd;
    double      zSpd;
    
    char        status[16];
    uint64_t    intruderHex;
    char        resolution[16];
    double      resValue;
    
    uint32_t    CRC;
    
    //Default constructor
    TCAS_msg();
    //AC_state constructor
    TCAS_msg(AC_state state);
    //Constructor fed with all the relevant data
    TCAS_msg(AC_state state, TCAS_state situation);
    //The implicit copy constructor is good enough for us

    //Simple function to return the proper size needed for 
    //messages (124 bytes)
    static int nonPaddedSize();

    void updateOwnStatus(AC_state state);
    void updateTCASStatus(TCAS_state state);

    std::string toString();


};

class broadcast_socket
{
    private:
    
    //Networking variables
    int sock_fd;
    struct sockaddr_in ownAddr;
    struct sockaddr_in broadcastAddr;
    struct sockaddr* sendAddr;
    unsigned int sendAddrSize;

    bool continueSend = true;
    bool continueRecv = true;

    //Determines when the next send should happen
    //std::chrono::time_point<std::chrono::high_resolution_clock> nextSend;
    std::chrono::high_resolution_clock::time_point nextSend;

    //Data buffers
    TCAS_msg stagedMsg;
    bool msgUpdated = false;
    bool msgInitialized = false;

    TCAS_msg tempRemoteMsg;


    std::thread sendThread; //Handles send and other timed tasks
    std::thread recvThread; //Waits for incoming broadcasts

    std::mutex ownStateMutex;
    std::mutex targetsMutex;
    
    void sendThreadFunction();
    void recvThreadFunction();
    
    public:
    
    broadcast_socket(int port);
    ~broadcast_socket();
    
    //Initialize everything
    void initializeStatus(AC_state ownState, TCAS_state tcasSituation);
    //Update our A/C's parameters without changing TCAS data
    void updateStatus(AC_state ownState);
    //Update everything
    void updateStatus(AC_state ownState, TCAS_state tcasSituation);
    //Get a vector of all other A/C statuses
    int getUpdatedTargetsStatus(std::vector<AC_state>& targetsStatus,
                                std::vector<TCAS_state>& targetsTCAS);
    
    
    
};