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
        UpdateTargetStates();
        Actual_TCAS();
        UpdateOwnState();
        printState(own_State_sim.getCurrentState());
        Radar_update(own_State_sim.getCurrentState(), own_TCAS_State, targetStates, target_TCAS_States);
        std::cout << "Targets list size: " << targetStates.size() << std::endl;
    }
    
}

void TCAS_sim::UpdateOwnState()
{
    own_State_sim.advanceToNow();
    socket_ptr->updateStatus(own_State_sim.getCurrentState(), own_TCAS_State);
}

void TCAS_sim::UpdateTargetStates(){
    
    socket_ptr->getUpdatedTargetsStatus(targetStates, target_TCAS_States);
    
}

//Check if which_target is or is going to be within min_safe_distance of us
//returns true if a resolution was taken
//CHECK THAT IT'S NOT POSSIBLE TO ENTER RESOLUTION STATUS WITHOUT ACTUALLY CHOOSING A RESOLUTION
bool TCAS_sim::resolve(int which_target){
    
    double time_to_approach;

    if(!analyse_collision_danger(which_target, time_to_approach))
        return false;
        
    cout << "########## Collision danger detected ###########" << endl;
        
    //If between 25s and 40s, enter ADVISORY status (no change on the course for now)
    //If within 25s, enter RESOLVING status, choosing climb or descent based on the rules in the pptx
        
    if(time_to_approach < Resolution_Time){
        
        //If target is resolving, and we don't have a resolution already implemented or lack priority, complement its resolution. Otherwise, keep our resolution. 
        //cout << "----->> " << own_State_sim.AC_ID << " ; " << targetStates[which_target].AC_ID << "; " << (strncmp( target_TCAS_States[which_target].status, "RESOLVING",16) == 0 and (strncmp(own_TCAS_State.status, "RESOLVING", 16)!=0 or  own_State_sim.AC_ID<targetStates[which_target].AC_ID)) << "<<------"   << endl;
        //cout << "----->> " << target_TCAS_States[which_target].status << " ; " << strncmp( target_TCAS_States[which_target].status, "RESOLVING",16) << endl;
        if(strncmp( target_TCAS_States[which_target].status, "RESOLVING",16) == 0 and (strncmp(own_TCAS_State.status, "RESOLVING", 16)!=0 or  own_State_sim.AC_ID<targetStates[which_target].AC_ID)){
            
            cout << "Complementing" << endl;
            
            if( strncmp( target_TCAS_States[which_target].resolution, "CLIMB",16)==0 ){
                //cout << "The other one was climbing. This one will now descent." << endl;
                strncpy(own_TCAS_State.resolution, "DESCENT", 16);
                own_State_sim.set_mode(DESCENT);
                
            
            } else if( strncmp( target_TCAS_States[which_target].resolution, "DESCENT",16)==0 ){

                strncpy(own_TCAS_State.resolution, "CLIMB", 16);
                own_State_sim.set_mode(CLIMB);
                
            }
            //If target is not resolving, decide on the resolution based on altitude
            else{
                double target_pos_xyz[3];
                double    own_pos_xyz[3];
                cout << "Aircraft ID " << targetStates[which_target].AC_ID << "Sent an invalid resolution" << endl;
            }
        }
        //Target isnt resolving
        else{
            strncpy(own_TCAS_State.resolution, "DESCENT", 16);
            own_State_sim.set_mode(DESCENT);
        }
        
        strncpy(own_TCAS_State.status, "RESOLVING", 16);
        own_TCAS_State.res_value = 5; //m/s
        own_TCAS_State.intruder_hex = targetStates[which_target].AC_ID;
        
        return true;
    }
    
    if(time_to_approach < Advisory_Time){
        
        strncpy(own_TCAS_State.status, "ADVISORY", 16);
        own_TCAS_State.intruder_hex = targetStates[which_target].AC_ID;
        
        return true; //?
    }
    
    return false;
}

void TCAS_sim::Actual_TCAS(){
    
    if(strncmp(own_TCAS_State.status, "CLEAR",16) == 0){
        
        //check if any target is or is going to be within min_safe_distance of us. 
        
        for(unsigned int i=0; i<targetStates.size(); i++){
            if(resolve(i))
                break;
        }
    
    }else if(strncmp(own_TCAS_State.status, "ADVISORY",16) == 0){
        
        //Same as above, but only for the current intruder
        for(unsigned int i=0; i<targetStates.size(); i++){
            if(own_TCAS_State.intruder_hex == targetStates[i].AC_ID){
                if( !resolve(i) ){
                    strncpy(own_TCAS_State.status, "CLEAR", 16);
                }
                return;
            }
        }
        
    }else if(strncmp(own_TCAS_State.status, "RESOLVING",16) == 0){
        
        //Same as above, but only for the current intruder
        for(unsigned int i=0; i<targetStates.size(); i++){
            if(own_TCAS_State.intruder_hex == targetStates[i].AC_ID){
                //check if we need to continue resolving. Calling resolve will also correct the resolution to whatever is compatible with the intruder
                if( !resolve(i) ){
                    //If not, set status to RETURNING, and put the controls at normal altitude
                    strncpy(own_TCAS_State.status, "RETURNING", 16);
                    own_State_sim.set_mode(CRUISE);
                }
                return;
            }
        }
        
        
    }else if(strncmp(own_TCAS_State.status, "RETURNING",16) == 0){
        //If the aircraft has already reached the normal altitude (within a few metres), set status to CLEAR
        if( own_State_sim.at_h_ref){
            strncpy(own_TCAS_State.status, "CLEAR", 16);
        }
        
    }else{
        cout << "Current TCAS status is invalid" << endl;
        exit(1);
    }
    
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

