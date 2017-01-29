#include "TCAS_sim.h"

TCAS_sim::TCAS_sim(AC_sim new_State_sim, broadcast_socket* new_socket_ptr){
    
    own_State_sim = new_State_sim;
    socket_ptr = new_socket_ptr;
    
    new_socket_ptr->initializeStatus(own_State_sim.getCurrentState(), own_TCAS_State);
    
    sim_thread = std::thread(&TCAS_sim::sim_thread_fn, this);
    
}

void TCAS_sim::sim_thread_fn(){
    
    while(1){
        usleep(900e3);
        UpdateOwnState();
        UpdateTargetStates();
        printState(own_State_sim.getCurrentState());
        Radar_update(own_State_sim.getCurrentState(), targetStates);
        std::cout << "Targets list size: " << targetStates.size() << std::endl;
    }
    
}

void TCAS_sim::UpdateOwnState()
{
    own_State_sim.advanceToNow();
    socket_ptr->updateStatus(own_State_sim.getCurrentState());
}

void TCAS_sim::UpdateTargetStates(){
    
    socket_ptr->getUpdatedTargetsStatus(targetStates, target_TCAS_States);
    
}


bool TCAS_sim::analyse_collision_danger(const int which_target, double &time_to_approach){
    
    AC_state own_state = own_State_sim.getCurrentState();
    
    //relative position
    double P[3] = {targetStates[which_target].x_pos - own_state.x_pos, 
                  targetStates[which_target].y_pos - own_state.y_pos, 
                  targetStates[which_target].z_pos - own_state.z_pos};
    
    //relative velocity
    double V[3] = {targetStates[which_target].x_spd - own_state.x_spd, 
                  targetStates[which_target].y_spd - own_state.y_spd, 
                  targetStates[which_target].z_spd - own_state.z_spd};    
    //
    
    double P_i_V = internal_product(P,V);
    double R2 = internal_product(P,P);
    double R = sqrt(R2);
    
    //No danger if target is distancing itself from us, and the distance is not dangerous at the moment.
    if(P_i_V > 0 && R>min_safe_distance){
        time_to_approach = 1e99; //large number
        return false;
    }
    
    //If the aircraft is already too close, well, it's too close
    if(R < min_safe_distance){
        time_to_approach = 0;
        return true;
    }
    
    //Evaluating weather the target will enter the sphere of radius min_safe_distance around us
    double V2 = internal_product(V,V);
    double Discriminant = P_i_V*P_i_V - V2*(R2-min_safe_distance*min_safe_distance);
    if(Discriminant < 0){
        time_to_approach = 1e99; //large number
        return false;
    }
    
    //Now computing the time until collision
    time_to_approach = (-P_i_V-sqrt(Discriminant))/V2;
    return true;
}

double internal_product(double A[3], double B[3]){
    
    double ret = 0.0;
    
    for(int i=0; i<3; i++)
        ret += A[i]*B[i];
    
    return ret;
}

void printState (AC_state state)
{
    cout << "Aircraft ID: " << state.AC_ID << endl;
    
    double P_xyz[3] = {state.x_pos, state.y_pos, state.z_pos};
    double P_llh[3];
    
    xyz_to_llh(P_xyz, P_llh);
    
    cout << "Position: ";
    for(int i=0; i<2; i++)
        cout << P_llh[i]*180/pi << "º ";
    cout << P_llh[2] << "m " << endl;
    
    double V_xyz[3] = {state.x_spd, state.y_spd, state.z_spd};;
    double V_enu[3];
    xyz_to_enu(V_xyz, P_llh[0], P_llh[1], V_enu);
    
    cout << "Velocity: ";
    for(int i=0; i<3; i++)
        cout << V_enu[i] << " ";
    cout << "(m/s)" << endl;
    
    /*cout << "Position: " << state.x_pos << "; " << state.y_pos << "; " << state.z_pos << endl;
    cout << "Velocity: " << state.x_spd << "; " << state.y_spd << "; " << state.z_spd << endl;*/
}

