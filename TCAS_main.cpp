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
#include "TCAS_defs.h"
#include "TCAS_comms.h"
#include "AC_sim.h"
#include "Radar.h"
void printState (AC_state state);   //Move or delete this

using std::cout; using std::endl;
 
int main(int argn, char *argv[])
{
    std::cout << "TCAS simulator Group C" << std::endl;
    std::cout << "Initializing..." << std::endl;
    
    
    uint64_t ownID = OWN_HEX;
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
            std::string linebuffStr = std::string(linebuff);
            if (!(linebuffStr == TCAS_INIT_FILE_HDR))
            {
                //TODO: Wire these variables in
                
                initfile >> ownID;
                initfile >> latInit;
                initfile >> lonInit;
                initfile >> hInit;
                initfile >> headInit;
                initfile >> spdInit;
                
                latInit *= pi/180;
                lonInit *= pi/180;
                headInit *= pi/180;
            }

        }
    }

    
    //Initialize Radar
    Radar_initialize();
    
    //Initialize networking
    broadcast_socket transSocket(10505);
    
    //TODO - Initialize own model

    AC_state ownInitState(ownID, latInit, lonInit, hInit);
    AC_sim ownAircraft(ownInitState);
    ownAircraft.set_controls(spdInit, hInit, headInit);
    TCAS_sim The_Simulator(ownAircraft, &transSocket);
    

    //This stuff should go in a separate thread
    //managed by an object
    
    while(1);
    
    //TODO - Cleanup
}


