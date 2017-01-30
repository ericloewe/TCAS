#include "AC_sim.h"

AC_sim::AC_sim()
{
}

AC_sim::AC_sim(AC_state initState)
{
    state = initState;
    mode = CRUISE;
    AC_ID = state.AC_ID;
}


void AC_sim::advanceToNow()
{
    std::chrono::high_resolution_clock::time_point curr_time = std::chrono::high_resolution_clock::now();
    milliseconds delta_t_chrono = duration_cast<milliseconds>(curr_time - state.time_of_issue);
    state.time_of_issue = curr_time;
    
    double delta_t = (double)delta_t_chrono.count()/1000.0;
    
    runge_kutta_4(delta_t, 0.010);
    
    //Unnecessary if second-order model gets implemented (but should be harmless)
    double f_val[6];
    f(state, f_val);
    state.x_spd = f_val[0];
    state.y_spd = f_val[1];
    state.z_spd = f_val[2];
}

AC_state AC_sim::getCurrentState(){
    advanceToNow();
    return state;
}

void AC_sim::runge_kutta_4 (double delta_t, double t_step)
{
    double k[5][6];
    AC_state aux_state;
    
    for(double t=0; t<delta_t; t+=t_step)
    {
        aux_state = state;
        
        f(aux_state, k[0]);
        euler_step(aux_state, k[0], t_step/2.0);
        f(aux_state, k[1]);
        euler_step(aux_state, k[1], t_step/2.0);
        f(aux_state, k[2]);
        euler_step(aux_state, k[2], t_step);
        f(aux_state, k[3]);
        
        for(int j=0; j<6; j++)
        {
            k[4][j] = k[0][j] + 2*k[1][j] + 2*k[2][j] + k[3][j];
        }
        
        
        euler_step(state, k[4], t_step/6.0);
    }
}

/** Linear
void AC_sim::f(const AC_state now_state, double f_value[6])
{
    double aux[6] = {now_state.x_spd, now_state.y_spd, now_state.z_spd, 0,0,0};
    
    for(int i=0; i<6; i++)
    {
        f_value[i] = aux[i];
    }
}*/

void AC_sim::f(const AC_state now_state, double f_value[6])
{
    double Pos_xyz[3] = {now_state.x_pos, now_state.y_pos, now_state.z_pos};
    double Pos_llh[3];
    xyz_to_llh(Pos_xyz, Pos_llh);
    
    //Consider this values in degrees for now
    double K = 0.1; //degrees per metre
    double h_obj = h_ref;
    
    if(mode==CLIMB)
        h_obj+=Altitude_Variation;  
        
    if(mode==DESCEND)
        h_obj-=Altitude_Variation;
    
    if(mode==CRUISE && abs(h_ref-Pos_llh[2])<10)
        at_h_ref = true;
    else
        at_h_ref = false;
    
    
    double climb_angle = K*(h_obj - Pos_llh[2]);
    
    //~ cout << h_ref << " " << Pos_llh[2] << " ";
    //~ cout << "-->Climb angle: " << climb_angle << endl;
    
    //limitting climb angle
    if(climb_angle > Max_Climb_Angle)
        climb_angle = Max_Climb_Angle;
    if(climb_angle <-Max_Climb_Angle)
        climb_angle = -Max_Climb_Angle;
        
        
    //Now converting to radians
    climb_angle *= pi/180;
    
    double V_aed[3] = {azimuth, climb_angle, V};
    double V_enu[3];
    aed_to_enu(V_aed, V_enu);
    
    double V_xyz[3];
    enu_to_xyz(V_enu, Pos_llh[0], Pos_llh[1], V_xyz);
    
    //double aux[6] = {now_state.x_spd, now_state.y_spd, now_state.z_spd, 0,0,0};
    
    for(int i=0; i<3; i++)
        f_value[i] = V_xyz[i];
    
    for(int i=3; i<6; i++)
        f_value[i] = 0;
}

void AC_sim::euler_step(AC_state &now_state, double f_value[6], double time_step)
{
    now_state.x_pos += f_value[0]*time_step;
    now_state.y_pos += f_value[1]*time_step;
    now_state.z_pos += f_value[2]*time_step;
    now_state.x_spd += f_value[3]*time_step;
    now_state.y_spd += f_value[4]*time_step;
    now_state.z_spd += f_value[5]*time_step;
}


void AC_sim::set_controls(double new_V, double new_h_ref, double new_azimuth)
{
    advanceToNow();
    
    V = new_V;
    h_ref = new_h_ref;
    azimuth = new_azimuth;
}

void AC_sim::set_mode(int new_mode){
    mode = new_mode;
}

