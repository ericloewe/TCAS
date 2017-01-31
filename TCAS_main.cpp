/* 
 *  Copyright notice
 *
 *
 *
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
void printState (AC_state state);   //Move or delete this

using std::cout; using std::endl;
 
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
                cout << "Wrong file header" << endl;
            }

        }
        else
        {
            cout << "Invalide file name" << endl;
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
        

        
        //DEBUG
        printStatusDisp(ownID, 38.23, -9, 3500, 124, 200, 0.001, "CLEAR", true, 5);
        std::cout << std::endl;
        

        


        std::this_thread::sleep_until(nextRefresh);




    }
    
    //TODO - Cleanup
}


