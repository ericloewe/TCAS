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

#include "TCAS_sim.h"

TCAS_sim::TCAS_sim(AC_sim new_State_sim, broadcast_socket* new_socket_ptr){
    
    own_State_sim = new_State_sim;
    socket_ptr = new_socket_ptr;
    
    new_socket_ptr->initializeStatus(own_State_sim.getCurrentState(), own_TCAS_State);
    
    sim_thread = std::thread(&TCAS_sim::sim_thread_fn, this);
    
}



/*
 *  Getter for our own TCAS state
 *
 *  Useful for UI work
 */
TCAS_state TCAS_sim::get_own_TCAS_State()
{
    return own_TCAS_State;
}

std::vector<AC_state> TCAS_sim::get_targetStates()
{
    return targetStates;
}


void TCAS_sim::sim_thread_fn(){
    
    int counter = 0;
    
    std::chrono::high_resolution_clock::time_point nextTick;
    nextTick = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::duration oneSecond(std::chrono::duration<long long>(1));

    while(continueLoop){
        nextTick += oneSecond;
        UpdateTargetStates();
        Actual_TCAS();
        UpdateOwnState();
        Radar_update(own_State_sim.getCurrentState(), own_TCAS_State, targetStates, target_TCAS_States);
        
        //Debug
        /*if(counter%3==0){
            printState(own_State_sim.getCurrentState());
            std::cout << "Targets list size: " << targetStates.size() << std::endl;
        }*/
        counter++;

        std::this_thread::sleep_until(nextTick);
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
//returns true if a resolution (or advisory ?) was taken
//CHECK THAT IT'S NOT POSSIBLE TO ENTER RESOLUTION STATUS WITHOUT ACTUALLY CHOOSING A RESOLUTION
bool TCAS_sim::resolve(int which_target){
    
    double time_to_approach;

    if(!analyse_collision_danger(which_target, time_to_approach))
        return false;
        
    //cout << "########## Collision danger detected ###########" << endl;
        
    //If between 25s and 40s, enter ADVISORY status (no change on the course for now)
    //If within 25s, enter RESOLVING status, choosing climb or descent based on the rules in the pptx
        
    if(time_to_approach < Resolution_Time){
        
        //If target is resolving, and we don't have a resolution already implemented or lack priority, complement its resolution. Otherwise, keep our resolution. 
        if(strncmp( target_TCAS_States[which_target].status, "RESOLVING",16) == 0 ){
            
            if(strncmp(own_TCAS_State.status, "RESOLVING", 16)!=0 or  own_State_sim.AC_ID<targetStates[which_target].AC_ID){
                //cout << "Complementing" << endl;
                
                if( strncmp( target_TCAS_States[which_target].resolution, "CLIMB",16)==0 ){
                    //cout << "The other one was climbing. This one will now descent." << endl;
                    strncpy(own_TCAS_State.resolution, "DESCEND", 16);
                    own_State_sim.set_mode(DESCEND);
                    
                
                } else if( strncmp( target_TCAS_States[which_target].resolution, "DESCEND",16)==0 ){

                    strncpy(own_TCAS_State.resolution, "CLIMB", 16);
                    own_State_sim.set_mode(CLIMB);
                    
                }
            
                else{                
                    cout << "Aircraft ID " << targetStates[which_target].AC_ID << "Sent an invalid resolution" << endl;
                }
            }
        }
        //Target isnt resolving - decide on our resolution based on altitude
        else{
            
            AC_state target_state = targetStates[which_target];
            AC_state own_state = own_State_sim.getCurrentState();
            
            double target_pos_xyz[3] = {target_state.x_pos, target_state.y_pos, target_state.z_pos};
            double own_pos_xyz[3] = {own_state.x_pos, own_state.y_pos, own_state.z_pos};
            
            double target_pos_llh[3];
            double    own_pos_llh[3];
            
            xyz_to_llh(target_pos_xyz, target_pos_llh);
            xyz_to_llh(own_pos_xyz, own_pos_llh);
            
            if(own_pos_llh[2] <= target_pos_llh[2]){
                strncpy(own_TCAS_State.resolution, "DESCEND", 16);
                own_State_sim.set_mode(DESCEND);
            }else{
                strncpy(own_TCAS_State.resolution, "CLIMB", 16);
                own_State_sim.set_mode(CLIMB);                
            }
            
            
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
        //check if any target is or is going to be within min_safe_distance of us. 
        for(unsigned int i=0; i<targetStates.size(); i++){
            if(resolve(i))
                break;
        }
    }else{
        cout << "Current TCAS status is invalid" << endl;
        exit(1);
    }
    
}



bool TCAS_sim::analyse_collision_danger(const int which_target, double &time_to_approach){
    
    AC_state own_state = own_State_sim.getCurrentState();
    
    //std::chrono::high_resolution_clock::time_point curr_time = std::chrono::high_resolution_clock::now();
    milliseconds delta_t_chrono = duration_cast<milliseconds>(own_state.time_of_issue - targetStates[which_target].time_of_issue);
    
    double issue_delay = (double)delta_t_chrono.count()/1000.0;
    
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
    
    double Danger_Range = min_safe_distance;
    
    //hysteresis to avoid flickering between resolution and returning
    if(strncmp(own_TCAS_State.status, "RESOLVING", 16)==0)
        Danger_Range = min_return_distance;
    
    //No danger if target is distancing itself from us, and the distance is not dangerous at the moment.
    if(P_i_V > 0 && R>Danger_Range){
        time_to_approach = 1e99; //large number
        return false;
    }
    
    //If the aircraft is already too close, well, it's too close
    if(R < Danger_Range){
        time_to_approach = 0;
        return true;
    }
    
    //Evaluating weather the target will enter the sphere of radius Danger_Range around us
    double V2 = internal_product(V,V);
    double Discriminant = P_i_V*P_i_V - V2*(R2-Danger_Range*Danger_Range);
    if(Discriminant < 0){
        time_to_approach = 1e99; //large number
        return false;
    }
    
    //Now computing the time until collision
    time_to_approach = (-P_i_V-sqrt(Discriminant))/V2 - issue_delay;
    return true;
}

double internal_product(double A[3], double B[3]){
    
    double ret = 0.0;
    
    for(int i=0; i<3; i++)
        ret += A[i]*B[i];
    
    return ret;
}




/*
 *          Group C TCAS Project
 *  https://github.com/ericloewe/TCAS
 *
 *  Sistemas Aviónicos Integrados 2016/2017
 *  Instituto Superior Técnico
 *
 *  Copyright 2017 
 *  Simão Marto     75326
 *  Eric Loewenthal 75848
 *  João Martins    76964
 *
 *  This software is licensed under the terms of the 
 *  GNU General Public License version 3.
 *
 *
 *  This copyright notice is here to help spot dirty thieves
 *
 */


//Debug
void printState (AC_state state)
{
    cout << endl;
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
    cout << endl;
    /*cout << "Position: " << state.x_pos << "; " << state.y_pos << "; " << state.z_pos << endl;
    cout << "Velocity: " << state.x_spd << "; " << state.y_spd << "; " << state.z_spd << endl;*/
}

//Simple getter
AC_sim TCAS_sim::getAC_sim()
{
    return own_State_sim;
}

