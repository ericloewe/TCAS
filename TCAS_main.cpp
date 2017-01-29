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

    double xinit = 0;
    double yinit = 0;
    double zinit = 0;
    
    double xdotinit = 200;
    double ydotinit = 0;
    double zdotinit = 0;

    uint64_t ownID;

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
                double latInit;
                double lonInit;
                double hInit;

                double headInit;
                double spdInit;

                
                initfile >> latInit;
                initfile >> lonInit;
                initfile >> hInit;
                initfile >> headInit;
                initfile >> spdInit;
            }

        }
    }
    else if (argn == 3)
    {
        if (!argList[1].compare("-id"))
        {
            std::stringstream tempStr = std::stringstream(argList[2]);
            uint64_t inID;
            tempStr >> inID;

            if (inID != 0)
            {
                ownID = inID;
                std::cout << "Using ID " << inID << " from CLI." << std::endl;
            }
            else
            {
                ownID = OWN_HEX;
            }
        }
        else
        {
            ownID = OWN_HEX;
        }
    }
    else
    {
        ownID = OWN_HEX;
    }
    
    
    Radar_initialize();
    //TODO - Acquire starting coordinates
    
    //TODO - Initialize own model
    //AC_state ownInitState(own_hex, xinit, yinit, zinit, xdotinit, ydotinit,
    //                        zdotinit);
    
    AC_state ownInitState(ownID, 0, 0, 1000); //1000m above S.Tomé e Príncipe
    TCAS_state ownInitTCAS = TCAS_state();
    
    AC_sim ownAircraft(ownInitState);
    ownAircraft.set_controls(270, 0, pi/2);

    //TODO - Initialize networking

    broadcast_socket transSocket(10505);

    transSocket.initializeStatus(ownInitState, ownInitTCAS);

    //This stuff should go in a separate thread
    //managed by an object
    

    AC_state ownState;
    
    while(1)
    {
        
        ownState = ownAircraft.getCurrentState();
        transSocket.updateStatus(ownState);
        printState(ownState);
        sleep(1);
        
        
        //Radar_update(ownAircraft, 

        
    }
    
    //TODO - Initialize TCAS simulator
    
    //TODO - Initialize TCAS resolver
    
    //TODO - Graphical visualization
    
    //TODO - Cleanup
}

void printState (AC_state state)
{
    cout << "Aircraft ID: " << state.AC_ID << endl;
    
    double P_xyz[3] = {state.x_pos, state.y_pos, state.z_pos};
    double P_llh[3];
    
    xyz_to_llh(P_xyz, P_llh);
    
    cout << "Position: ";
    for(int i=0; i<2; i++)
        cout << P_llh[i]*180/pi << "º ";
    cout << P_llh[2] << "m " << endl;
    
    double V_xyz[3] = {state.x_spd, state.y_spd, state.z_spd};;
    double V_enu[3];
    xyz_to_enu(V_xyz, P_llh[0], P_llh[1], V_enu);
    
    cout << "Velocity: ";
    for(int i=0; i<3; i++)
        cout << V_enu[i] << " ";
    cout << "(m/s)" << endl;
    
    /*cout << "Position: " << state.x_pos << "; " << state.y_pos << "; " << state.z_pos << endl;
    cout << "Velocity: " << state.x_spd << "; " << state.y_spd << "; " << state.z_spd << endl;*/
}