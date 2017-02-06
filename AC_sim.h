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


AC_sim: Definitions related to the simulation of our own aircraft

 */

#pragma once

#include "TCAS_defs.h"
#include "Navigation.h"

#include <chrono>
#include <iostream>
#include <cmath>

using namespace std;

const double Altitude_Variation = 300; //m
const double Max_Climb_Angle = 2; //º

enum {CRUISE=0, CLIMB, DESCEND};

//typedef std::chrono::duration<long long int> milliDuration;
using namespace std::chrono;

class AC_sim
{
private:
    
    AC_state state;
    int mode;
    
public:
    
    uint64_t AC_ID;
    
    //Constructor with initial state
    AC_sim(AC_state initState);
    AC_sim();
    
    //Get the current aircraft simulation state
    //Used to update the TCAS simulation and to acquire
    //data to send over the network
    AC_state getCurrentState();
    
    void set_controls(double new_V, double new_h_ref, double new_azimuth);
    void set_mode(int new_mode);
    
    //bool cmdInputs();
    void advanceToNow();
    
    bool at_h_ref;

    AC_state getAC_state();
    
private:
    void step(milliseconds stepDuration);
    void runge_kutta_4 (double delta_t, double t_step);
    void f(const AC_state now_state, double f_value[6]);
    void euler_step(AC_state &now_state, double f_value[6], double time_step);
    
    

    double V;
    double h_ref;
    double azimuth;
};
