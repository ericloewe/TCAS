#pragma once

#include "TCAS_defs.h"
#include "Navigation.h"

#include <chrono>
#include <iostream>

using namespace std;

enum {CRUISE=0, CLIMB, DESCENT, TURN};

//typedef std::chrono::duration<long long int> milliDuration;
using namespace std::chrono;

class AC_sim
{
    private:
    
    AC_state state;
    int mode;
    
    
    
    public:
    
    //Constructor with initial state
    AC_sim(AC_state initState);
    
    //Get the current aircraft simulation state
    //Used to update the TCAS simulation and to acquire
    //data to send over the network
    AC_state getCurrentState();
    
    void set_controls(double new_V, double new_h_ref, double new_azimuth);
    
    //bool cmdInputs();
    
    private:
    void advanceToNow();
    void step(milliseconds stepDuration);
    void runge_kutta_4 (int mode, double delta_t, double t_step);
    void f(const AC_state now_state, double f_value[6]);
    void euler_step(AC_state &now_state, double f_value[6], double time_step);

    
    double V;
    double h_ref;
    double azimuth;
};