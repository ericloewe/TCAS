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


TCAS_main: 
    Starts the program, reads the init file, constructs assorted objects, etc.

 */


#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <string>
#include <string.h>
#include "TCAS_defs.h"
#include "TCAS_comms.h"
#include "AC_sim.h"
#include "Radar.h"
#include "TCAS_CLI.h"
//void printState (AC_state state);   //Move or delete this

//using std::cout; using std::endl;
 
int main(int argn, char *argv[])
{
    std::cout << "TCAS simulator Group C" << std::endl;
    std::cout << "Initializing..." << std::endl;
    
    //char tcasInitHeaderStr[] = TCAS_INIT_FILE_HDR; //Debug
    
    uint64_t ownID = OWN_HEX;

    int remotePort  = TCAS_DEFAULT_PORT;
    int localPort   = TCAS_DEFAULT_PORT;

    double latInit = 0;
    double lonInit = 0;
    double hInit = 1000;

    double headInit = 0;
    double spdInit = 270;

    std::ifstream initfile; 
    
    std::vector<std::string> argList;
    for (int i = 0; i < argn; i++)
    {
        std::string argNoI = argv[i];
        argList.push_back(argNoI);   
    }
    
    if (argn == 2)  //Read init file
    {
        initfile = std::ifstream(argList[1]);
        if (initfile.good())
        {
            char linebuff[128];
            initfile.getline(linebuff, 128);
            //std::string linebuffStr = std::string(linebuff);
            int temp = strncmp(linebuff, TCAS_INIT_FILE_HDR, TCAS_INIT_FILE_HDR_LEN);
            std::cout << "strncmp returns: " << temp << std::endl;
            if (temp == 0)
            {
                initfile >> remotePort;
                initfile.getline(linebuff, 128);
                initfile >> localPort;
                initfile.getline(linebuff, 128);
                
                initfile >> ownID;
                initfile.getline(linebuff, 128);
                initfile >> latInit;
                initfile.getline(linebuff, 128);
                initfile >> lonInit;
                initfile.getline(linebuff, 128);
                initfile >> hInit;
                initfile.getline(linebuff, 128);
                initfile >> headInit;
                initfile.getline(linebuff, 128);
                initfile >> spdInit;
                initfile.getline(linebuff, 128);
                
                latInit *= pi/180;
                lonInit *= pi/180;
                headInit *= pi/180;
            }
            else
            {
                std::cout << "Wrong file header" << std::endl;
            }

        }
        else
        {
            std::cout << "Invalide file name" << std::endl;
        }
    }

    
    //Initialize Radar
    Radar_initialize();
    
    //Initialize networking
    broadcast_socket transSocket(remotePort, localPort);
    
    //TODO - Initialize own model

    AC_state ownInitState(ownID, latInit, lonInit, hInit);
    AC_sim ownAircraft(ownInitState);
    ownAircraft.set_controls(spdInit, hInit, headInit);
    TCAS_sim The_Simulator(ownAircraft, &transSocket);
    
    
    bool continueProgram = true;

    //CLI timer
    auto nextRefresh = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));
    
    //int i = 0;
    
    while(continueProgram)
    {
        nextRefresh += oneSecond;
        //auto nextRefreshTime = nextRefresh.time_since_epoch();

        //Issue a VT100 screen clear command [ <ESC>[2J ]
        fprintf(stdout, "%c%c%c%c", 0x1b, 0x5b, 0x32, 0x4a);
        //Issue a VT100 return to home command [ <ESC>[H ]
        fprintf(stdout, "%c%c%c", 0x1b, 0x5b, 0x48);

        std::cout << "                        ---===TCAS V01 Overview===---" << std::endl;
        std::cout << "Current status:" << std::endl;
        printStatusHeader();
        
        double ownLat, ownLon, ownAlt, ownHDG, ownTAS, ownVup;
        
        convertData(The_Simulator.getAC_sim().getAC_state(),
                    ownLat, ownLon, ownAlt, ownHDG, ownTAS, ownVup);

        //DEBUG
        printStatusDisp(ownID, ownLat, ownLon, ownAlt, ownHDG, ownTAS, ownVup);
        std::cout << std::endl << std::endl << "Targets: " << std::endl;
        printStatusHeaderLong();

        
        std::vector<AC_state> targets; //= //The_Simulator.get_targetStates();
        std::vector<TCAS_state> tgtTCAS; // = transSocket.
        std::vector<unsigned int> tgtTimeouts; 
        std::vector<bool> tgtCRC;
        
        int numOfTargets = transSocket.getUpdatedTargetsStatus(targets, tgtTCAS, 
                                                                tgtTimeouts, tgtCRC);

        for (int j = 0; j < numOfTargets; j++)
        {
            std::string statusStr = tgtTCAS[j].status;

            convertData(targets[j], ownLat, ownLon, ownAlt, ownHDG, ownTAS, ownVup);
            printStatusDisp(targets[j].AC_ID, ownLat, ownLon, ownAlt, ownHDG, ownTAS, ownVup,
                            statusStr, tgtCRC[j], tgtTimeouts[j]);
            std::cout << std::endl;
        }

        std::this_thread::sleep_until(nextRefresh);




    }
    
    //TODO - Cleanup
}


