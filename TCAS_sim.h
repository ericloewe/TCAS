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


TCAS_sim: The actual TCAS work happens here

 */

#pragma once

#include <unistd.h>
#include <vector>
#include <thread>
#include <string.h>
 
#include "TCAS_defs.h"
#include "AC_sim.h"
#include "TCAS_comms.h"
#include "Radar.h"
 
class TCAS_sim
{
public:
    const static int NumOfTargets = MAX_TARGETS; //Maximum???
    
    
private:
    
    AC_sim own_State_sim;
    TCAS_state own_TCAS_State;
    broadcast_socket* socket_ptr;
    
    std::vector<AC_state> targetStates;
    std::vector<TCAS_state> target_TCAS_States;
    
    //Used to communicate with the socket
    void UpdateOwnState();
    void UpdateTargetStates();
    
    //Function to have its own thread, and its thread handler
    void sim_thread_fn();
    std::thread sim_thread;
    
    //Performs the decisions characterstic of a TCAS system
    bool resolve(int which_target);
    void Actual_TCAS();
    const double min_safe_distance = 600; //m
    const double min_return_distance = 1000; //m
    const double Resolution_Time = 30; //s
    const double Advisory_Time = 40; //s

    bool continueLoop = true;
    
    
    bool analyse_collision_danger(const int which_target, double &time_to_approach); //returns weather this aircraft will be or is within min_safe_distance of our aircraft
    
    
public:

    TCAS_sim(AC_sim new_State_sim, broadcast_socket* new_socket_ptr);
    TCAS_state get_own_TCAS_State();
    std::vector<AC_state> get_targetStates();
    AC_sim getAC_sim();
    
};

double internal_product(double A[3], double B[3]);
void printState (AC_state state);
