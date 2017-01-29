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
    
    void sim_thread_fn();
    
    std::vector<AC_state> targetStates;
    std::vector<TCAS_state> target_TCAS_States;
    
    std::thread sim_thread;
    
public:

    TCAS_sim(AC_sim new_State_sim, broadcast_socket* new_socket_ptr);
    
    
    void UpdateOwnState();
    void UpdateTargetStates(); //Using const qualifier gives error
    
};
 
void printState (AC_state state);
