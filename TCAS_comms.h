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

    //Outgoing Data buffers
    TCAS_msg stagedMsg;
    bool msgUpdated = false;
    bool msgInitialized = false;

    //Incoming data buffers
    AC_state    targetsList[MAX_TARGETS];
    TCAS_state  targetsTCAS[MAX_TARGETS];
    TCAS_msg tempRemoteMsg; //Holds recv'd data
    std::chrono::high_resolution_clock::time_point recvTime;

    std::thread sendThread; //Handles send and other timed tasks
    std::thread recvThread; //Waits for incoming broadcasts

    std::mutex ownStateMutex;
    std::mutex targetsMutex;
    
    void sendThreadFunction();
    void recvThreadFunction();

    //Converts received messages to our internal format and adds
    //them to the list.
    bool processTarget(TCAS_msg tgtMsg, 
        std::chrono::high_resolution_clock::time_point recvTime);

    
    public:
    
    broadcast_socket(int destPort, int inPort);
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