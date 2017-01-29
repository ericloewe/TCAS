/*
 *  TCAS_sim.h
 */

#pragma once

#include <unistd.h>
#include <vector>
#include <thread>
 
#include "TCAS_defs.h"
#include "AC_sim.h"
#include "TCAS_comms.h"

 
class TCAS_sim
{
public:
    const static int NumOfTargets = 10; //Maximum???
    
    
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
    void Actual_TCAS();
    const double min_safe_distance = 1e3; //m
    bool analyse_collision_danger(const int which_target, double &time_to_approach); //returns weather this aircraft will be or is within min_safe_distance of our aircraft
    
    
public:

    TCAS_sim(AC_sim new_State_sim, broadcast_socket* new_socket_ptr);
    
};

double internal_product(double A[3], double B[3]);
void printState (AC_state state);
