#pragma once

#include <chrono>
#include <string>
#include <vector>

#define TCAS_DEFAULT_PORT 10505

#define OWN_HEX 216412359
#define MAX_TARGETS 16

#define DEFAULT_MSG_INIT_SIZE 256 //???
#define TCAS_MSG_LEN 124

#define TCAS_MSG_HEADER "ACIP TCAS   V01\0"
#define TCAS_MSG_STRLEN 16

#define TCAS_INIT_FILE_HDR "TCAS Simulation Init File Format V1"
#define TCAS_INIT_FILE_HDR_LEN 35


const uint64_t own_hex = OWN_HEX;

//Forward declarations
class AC_state;
class TCAS_state;


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


class AC_state
{
    friend class AC_sim; //to access the positions and velocities
    friend class TCAS_sim; //to access AC_ID
    friend void printState(AC_state state);
    friend void Radar_update(AC_state ownState, std::vector<AC_state> targetStates);
public:
    uint64_t AC_ID;
    
    double x_pos;
    double y_pos;
    double z_pos;
    
    double x_spd;
    double y_spd;
    double z_spd;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> time_of_issue;
    
    //Default constructor, marks as invalid state
    AC_state();
    AC_state(TCAS_msg message, std::chrono::high_resolution_clock::time_point recvTime);
    AC_state(uint64_t ID, double xpos, double ypos, double zpos,
                double xspd, double yspd, double zspd);
    AC_state(uint64_t ID, double Latitude, double Longitude, double Altitude); //velocity to be defined by commands            
    
    //Getters and setters
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
    TCAS_state(TCAS_msg message);
};

