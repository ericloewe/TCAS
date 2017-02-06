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


TCAS_defs: General definitions used throughout the project

 */

#include <iostream>
#include <cstring>
#include <sstream>
#include <boost/crc.hpp>
#include "TCAS_defs.h"
#include "Navigation.h"



/*
 *  updateTCASStatus - Update the message's own state fields
 */
void TCAS_msg::updateOwnStatus(AC_state state)
{    
    ac_id = state.getID();
    
    xPos = state.getX_pos();
    yPos = state.getY_pos();
    zPos = state.getZ_pos();
    
    xSpd = state.getX_spd();
    ySpd = state.getY_spd();
    zSpd = state.getZ_spd();
}



/*
 *  updateTCASStatus - Update the message's TCAS fields
 */
void TCAS_msg::updateTCASStatus(TCAS_state state)
{    
    strncpy(status, state.status, 16);
    strncpy(resolution, state.resolution, 16);
    intruderHex = state.intruder_hex;
    resValue = state.res_value;
}



/*  
 *  Constructor: Nothing is known.
 */
TCAS_msg::TCAS_msg()
{
    //Nothing to construct.
}



/*  
 *  Constructor: Use existing AC_state to initialize the message.
 *
 *      TCAS status data is not initialized
 *
 *      CRC32 is calculated only upon transmission
 */
TCAS_msg::TCAS_msg(AC_state state)
{
    //Set the correct header
    strncpy(header, TCAS_MSG_HEADER, TCAS_MSG_STRLEN);
    
    updateOwnStatus(state);
    
    //We don't have TCAS status data
    
    //CRC32 is calculated just before transmission
}



/*  
 *  Constructor: Use existing AC_state and TCAS_state to initialize 
 *                  the message.
 *
 *      CRC32 is calculated only upon transmission
 */
TCAS_msg::TCAS_msg(AC_state state, TCAS_state situation)
{
    //Set the correct header
    strncpy(header, TCAS_MSG_HEADER, TCAS_MSG_STRLEN);
    
    updateOwnStatus(state);
    updateTCASStatus(situation);
    
    //CRC32 is calculated just before transmission
}



/*  
 *  Due to compiler optimizations, sizeof(TCAS_msg) is
 *  128 bytes. The real message size is only 124 bytes.
 *  We can't just send four extra bytes willy-nilly.
 *  Well, we *could*, but other people would not be happy
 *  as we'd probably expose weird edge cases in their code...
 *
 *  Returns 124. Yes, that's it.
 */    
int TCAS_msg::nonPaddedSize()
{
    return TCAS_MSG_LEN;
}



/*
 *  TCAS_msg::toString()
 *
 *  Returns a pretty-format string to display
 *  a TCAS message to the user
 *
 */
 std::string TCAS_msg::toString()
 {
     std::stringstream output;
     
     output << header;
     output << std::endl << "Aircraft ID: ";
     output << ac_id << std::endl;
     output << "Position: ";
     output << (xPos) << "  ";
     output << (yPos) << "  ";
     output << (zPos) << std::endl;
     output << "Velocity: ";
     output << xSpd << "  ";
     output << ySpd << "  ";
     output << zSpd << std::endl;
     output << "Conflict info:" << std::endl;
     output << status << std::endl;
     output << "Intruder ID: ";
     output << intruderHex << std::endl;
     output << "Resolution: ";
     output << resolution << std::endl;
     output << "Res. Value: ";
     output << resValue << std::endl;

     return output.str();
 }



 /*
 *  Calculate the CRC32 for the TCAS_msg
 *  
 *  The CRC32 field of the TCAS_msg is ignored
 *  Returns the CRC32.
 */
uint32_t TCAS_msg::getCRC32()
{
    //Some trickery to get an array of bytes
    char *msgBuf = (char *)this;

    boost::crc_32_type  result;

    result.process_bytes(msgBuf, TCAS_MSG_BODY_LEN);

    return result.checksum();
}



//Default constructor
//Sets invalid AC_ID
AC_state::AC_state()
{
    AC_ID = 0;
}



/*
 *  Constructs AC_state from a TCAS_msg
 */
AC_state::AC_state(TCAS_msg message, std::chrono::high_resolution_clock::time_point recvTime)
{
    AC_ID = message.ac_id;

    x_pos = message.xPos;
    y_pos = message.yPos;
    z_pos = message.zPos;

    x_spd = message.xSpd;
    y_spd = message.ySpd;
    z_spd = message.zSpd;

    time_of_issue = recvTime;
}



//Standard constructor
//Sets all fields    
AC_state::AC_state(uint64_t ID, double xpos, double ypos, double zpos,
                double xspd, double yspd, double zspd)
{
    AC_ID = ID;
    
    x_pos = xpos;
    y_pos = ypos;
    z_pos = zpos;
    
    x_spd = xspd;
    y_spd = yspd;
    z_spd = zspd;
    
    time_of_issue = std::chrono::high_resolution_clock::now();
    
    //DEBUG
    //auto duration = time_of_issue.time_since_epoch();
    //std::cout << "High resolution timer: " << duration.count() << std::endl;
}



AC_state::AC_state(uint64_t ID, double Latitude, double Longitude, double Altitude)
{
    
    AC_ID = ID;
    
    double Pos_llh[3] = {Latitude, Longitude, Altitude};
    double Pos_xyz[3];
    llh_to_xyz(Pos_llh, Pos_xyz);
    
    x_pos = Pos_xyz[0];
    y_pos = Pos_xyz[1];
    z_pos = Pos_xyz[2];
    
    x_spd = 0;
    y_spd = 0;
    z_spd = 0;
    
    time_of_issue = std::chrono::high_resolution_clock::now();
}    




/*
 *  Getters
 */
uint64_t AC_state::getID()
{
    return AC_ID;
}
double AC_state::getX_pos()
{
    return x_pos;
}
double AC_state::getY_pos()
{
    return y_pos;
}
double AC_state::getZ_pos()
{
    return z_pos;
}
double AC_state::getX_spd()
{
    return x_spd;
}
double AC_state::getY_spd()
{
    return y_spd;
}
double AC_state::getZ_spd()
{
    return z_spd;
}
/*
 *  End getters
 */



/*
 *  Default constructor - creates an all-clear state
 */
TCAS_state::TCAS_state()
{
    strncpy(status, "CLEAR\0", 16);
    resolution[0] = '\0';
    intruder_hex = 0;
    res_value = 0;
}



/*
 *  Constructs TCAS_state from a TCAS_msg
 */
TCAS_state::TCAS_state(TCAS_msg message)
{
    strncpy(status, message.status, TCAS_MSG_STRLEN);
    strncpy(resolution, message.resolution, TCAS_MSG_STRLEN);
    intruder_hex = message.intruderHex;
    res_value = message.resValue;
} 



