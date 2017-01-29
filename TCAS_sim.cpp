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

