#pragma once

#include "TCAS_defs.h"
#include "Navigation.h"

#include <chrono>
#include <iostream>
#include <cmath>

using namespace std;

const double Altitude_Variation = 300; //m

enum {CRUISE=0, CLIMB, DESCENT};

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
    
    private:
    void step(milliseconds stepDuration);
    void runge_kutta_4 (double delta_t, double t_step);
    void f(const AC_state now_state, double f_value[6]);
    void euler_step(AC_state &now_state, double f_value[6], double time_step);
    
    double V;
    double h_ref;
    double azimuth;
};
