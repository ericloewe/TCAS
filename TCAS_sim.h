/*
 *  TCAS_sim.h
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
    const double Resolution_Time = 30; //s
    const double Advisory_Time = 40; //s
    
    
    bool analyse_collision_danger(const int which_target, double &time_to_approach); //returns weather this aircraft will be or is within min_safe_distance of our aircraft
    
    
public:

    TCAS_sim(AC_sim new_State_sim, broadcast_socket* new_socket_ptr);
    TCAS_state get_own_TCAS_State();
    AC_sim getAC_sim();
    
};

double internal_product(double A[3], double B[3]);
void printState (AC_state state);
