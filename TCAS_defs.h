#pragma once

#include <chrono>

#define OWN_HEX 216412359


const uint64_t own_hex = OWN_HEX;

class AC_state
{
    friend class AC_sim; //to access the positions and velocities
    friend class TCAS_sim; //to access AC_ID
    friend void printState(AC_state state);
    friend void Radar_update(AC_state ownState, AC_state *targetStates);
private:
    uint64_t AC_ID;
    
    double x_pos;
    double y_pos;
    double z_pos;
    
    double x_spd;
    double y_spd;
    double z_spd;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> time_of_issue;
    
   
    
public:
    //Default constructor, marks as invalid state
    AC_state();
    AC_state(uint64_t ID, double xpos, double ypos, double zpos,
                double xspd, double yspd, double zspd);
    AC_state(uint64_t ID, double Latitude, double Longitude, double Altitude); //velocity to be defined by commands            
    uint64_t getID();
    double getX_pos();
    double getY_pos();
    double getZ_pos();
    double getX_spd();
    double getY_spd();
    double getZ_spd();
    
};

class TCAS_state
{
public:
    char        status[16];
    char        resolution[16];
    
    uint64_t    intruder_hex;
    
    double      res_value;
    
    //Default constructor - creates an all-clear state
    TCAS_state();
};

