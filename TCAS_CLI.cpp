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


TCAS_CLI: Definitions for the CLI

 */


#include <iostream>
#include <string.h>
#include <iomanip>
#include <math.h>
#include <string.h>
#include "TCAS_defs.h"
#include "Navigation.h"

using namespace std;

void printStatusHeader()
{
    cout << "    Hex        Lat         Lon         Alt         HDG         TAS         Vup" << endl;
}

void printStatusHeaderLong()
{
    cout << "    Hex        Lat         Lon         Alt         HDG         TAS         Vup      Status      CRC32       Timeout" << endl;
}

void printStatusDisp(long long ac_hex, double lat, double lon, double altitude, double hdg, double TAS, double Vup)
{
    cout << fixed;
    cout.precision(4);
    cout << "   " << hex << setw(8) << ac_hex << "    ";
    cout << setw(7) << lat << "      " << setw(8) << lon << "  ";
    cout.precision(0);
    cout << setw(5) << altitude << "        ";
    cout.precision(1);
    cout << setw(5) << hdg << "       " << setw(4) << TAS << "       ";
    cout.precision(2);
    cout << setw(5) << Vup;
}

void printStatusDisp(long long ac_hex, 
                    double lat, double lon, double altitude, 
                    double hdg, double TAS, double Vup,
                    std::string status, bool CRCpass, int timeout)
{
    printStatusDisp(ac_hex, lat, lon, altitude, hdg, TAS, Vup);
    cout << "    ";
    if (status == "CLEAR")
    {
        cout << "CLEAR       ";
    }
    else if (status == "ADVISORY")
    {
        cout << "ADVISORY    ";
    }
    else if (status == "RESOLUTION")
    {
        cout << "RESOLVING  ";
    }
     else if (status == "RESOLVING")
    {
        cout << "RESOLVING  ";
    }
    else if (status == "RETURNING")
    {
        cout << "RETURNING   ";
    }
    else
    {
        cout << "???         ";
    }

    if (CRCpass)
    {
        cout << "OK          ";
    }
    else
    {
        cout << "FAIL!       ";
    }

    cout << setw(1) << timeout;
}



void convertData (AC_state state,
                 double& lat, double& lon, double& altitude,
                 double& HDG, double& TAS, double& Vup)
{    
    double P_xyz[3] = {state.x_pos, state.y_pos, state.z_pos};
    double P_llh[3];
    
    xyz_to_llh(P_xyz, P_llh);
    
    lat = P_llh[0] *180 / pi;
    lon = P_llh[1] * 180 / pi;
    altitude = P_llh[2];
    
    double V_xyz[3] = {state.x_spd, state.y_spd, state.z_spd};;
    double V_enu[3];
    xyz_to_enu(V_xyz, P_llh[0], P_llh[1], V_enu);
    
    TAS = sqrt(pow(V_enu[0], 2) + pow(V_enu[1], 2));
    Vup = V_enu[2];

    HDG = atan2(V_enu[0], V_enu[1]) * 180 / pi;
    if(HDG < 0)
        HDG += 360;
}
