/*
    Group C TCAS Project
https://github.com/ericloewe/TCAS

Sistemas Aviónicos Integrados 2016/2017
Instituto Superior Técnico

Copyright 2017 

Simão Marto     75326
Eric Loewenthal 75848
João Martins    76964

This software is licensed under the terms of the GNU General Public License version 3.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


TCAS_comms: Definitions related to networking

All actual networking happens here

 */

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
    unsigned int    timeout[MAX_TARGETS];
    bool        CRC32status[MAX_TARGETS];

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
    //Same as above, but with more data
    int getUpdatedTargetsStatus(std::vector<AC_state>& targetsStatusVector,
                                std::vector<TCAS_state>& targetsTCASVector,
                                std::vector<unsigned int>& targetsTimeoutVector,
                                std::vector<bool>& targetsCRC32StatusVector);
    

};