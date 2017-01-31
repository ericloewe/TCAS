#include "Radar.h"

int virtual_dev, physical_dev; 

bool centre_set = false;
double Centre_Latitude, Centre_Longitude;
double Centre_xyz[3];
double Centre_V_enu[3];

int background_colour, stuff_colour, advisory_colour, resolution_colour, returning_colour;

void Radar_initialize(){
        
    physical_dev = g2_open_X11 (W, H);
    virtual_dev = g2_open_vd ();
    g2_clear (virtual_dev);
    g2_attach (virtual_dev, physical_dev);
    g2_set_auto_flush (virtual_dev, 0);
    
    background_colour = g2_ink(physical_dev, 0, 0, 0);
    stuff_colour      = g2_ink(physical_dev, 0, 0.75, 0);
    advisory_colour   = g2_ink(physical_dev, 0.9, 0.9, 0.1);
    resolution_colour = g2_ink(physical_dev, 0.9, 0.1, 0.1);
    returning_colour  = g2_ink(physical_dev, 0.1, 0.1, 0.9);
    
    Radar_draw_background();
    g2_flush(virtual_dev);
}

void Radar_set_centre(AC_state centre_craft){
    
    //Setting the centre position (xyz, Latitude and Longitude)
    double pos_xyz[3] = {centre_craft.x_pos, centre_craft.y_pos, centre_craft.z_pos};
    for(int i=0; i<3; i++){
        Centre_xyz[i] = pos_xyz[i];
    }
    double Centre_llh[3];
    xyz_to_llh(Centre_xyz, Centre_llh);
    Centre_Latitude = Centre_llh[0];
    Centre_Longitude = Centre_llh[1];
    
    //Setting the centre velocity (enu)
    double vel_xyz[3] = {centre_craft.x_spd, centre_craft.y_spd, centre_craft.z_spd};
    xyz_to_enu(vel_xyz, Centre_Latitude, Centre_Longitude, Centre_V_enu);

    
    centre_set = true;
    
}

void Radar_draw_background(){
    g2_set_background(virtual_dev, background_colour);
    
    g2_pen(virtual_dev, stuff_colour);
    g2_filled_circle(virtual_dev, W/2, H/2, R);
    
    g2_pen(virtual_dev, background_colour);
    g2_filled_circle(virtual_dev, W/2, H/2, R-5);
    
    g2_pen(virtual_dev, stuff_colour);
    int N = 5;
    for(int i=1; i<N; i++)
        g2_circle(virtual_dev, W/2, H/2, R*i/N);
    
    double inv_sqrt2 = 0.70710678118;
    // g2_line(int dev, double x1, double y1, double x2, double y2)
    for(int i=-1; i<=1; i++){
        g2_line(virtual_dev, W/2-R, H/2+i, W/2+R, H/2+i);
        g2_line(virtual_dev, W/2+i, H/2-R, W/2+i, H/2+R);
    }
    g2_line(virtual_dev, (W/2-R*inv_sqrt2), (H/2-R*inv_sqrt2), (W/2+R*inv_sqrt2), (H/2+R*inv_sqrt2));
    g2_line(virtual_dev, (W/2-R*inv_sqrt2), (H/2+R*inv_sqrt2), (W/2+R*inv_sqrt2), (H/2-R*inv_sqrt2));
}

void Radar_draw_plane(double P_xyz[3], char ID_str[16]){
    
    if(!centre_set)
        _exit(1);
    
    double Dir_to_P[3];
    for(int i=0; i<3; i++)
        Dir_to_P[i] = P_xyz[i] - Centre_xyz[i];
    
    double Dir_to_P_ENU[3];
    xyz_to_enu(Dir_to_P, Centre_Latitude, Centre_Longitude, Dir_to_P_ENU);
    
    if( (Dir_to_P_ENU[0]*Dir_to_P_ENU[0] + Dir_to_P_ENU[1]*Dir_to_P_ENU[1]) > Range*Range)
        return;
    
    double Dir_to_P_AirFrame[2]; //Horizontal coords only
    
    double aux_V_enu[3];
    aux_V_enu[0] = Centre_V_enu[0]; aux_V_enu[1] = Centre_V_enu[1]; aux_V_enu[2] = 0; //This coordinates are horizontal only. vertical components of speed are ignored.
    double mag_V = sqrt(aux_V_enu[0]*aux_V_enu[0] + aux_V_enu[1]*aux_V_enu[1]);
    Dir_to_P_AirFrame[0] = internal_product(Dir_to_P_ENU, aux_V_enu)/mag_V; // Component along velocity
    
    aux_V_enu[0] = Centre_V_enu[1]; aux_V_enu[1] = -Centre_V_enu[0]; //The normal to this vector
    Dir_to_P_AirFrame[1] = internal_product(Dir_to_P_ENU, aux_V_enu)/mag_V; // Component to the right of velocity
    
    int x = (double)W/2.0 + (double)R*Dir_to_P_AirFrame[1]/Range;
    int y = (double)H/2.0 + (double)R*Dir_to_P_AirFrame[0]/Range;
    g2_filled_circle(virtual_dev, x, y, r);
    g2_set_font_size (virtual_dev, 10);
    g2_string(virtual_dev, x+r, y+r, ID_str);
    
    /*char pos_STR[32];
    double P_llh[3];
    xyz_to_llh(P_xyz, P_llh);
    sprintf(pos_STR, "%7.4fDeg %7.4fDeg %6.1fm", P_llh[0]*180.0/pi, P_llh[1]*180.0/pi, P_llh[2]);
    g2_string(virtual_dev, x+r, y-r, pos_STR);*/
}

int get_colour(char *TCAS_status){
    
    if(strncmp(TCAS_status, "CLEAR",16)==0)
        return stuff_colour;
    
    if(strncmp(TCAS_status, "ADVISORY",16)==0)
        return advisory_colour;
    
    if(strncmp(TCAS_status, "RESOLVING",16)==0)
        return resolution_colour;
    
    if(strncmp(TCAS_status, "RETURNING",16)==0)
        return returning_colour;
    
    return 0; //white
}

void Radar_update(AC_state ownState, TCAS_state own_TCAS_State, std::vector<AC_state> targetStates, std::vector<TCAS_state> target_TCAS_States){
    
    char ID_str[16];
    
    Radar_draw_background();
    
    g2_pen(virtual_dev, get_colour(own_TCAS_State.status));
    
    g2_set_font_size (virtual_dev, 60);
    g2_string(virtual_dev, 0, 0, own_TCAS_State.status);
    if(strncmp(own_TCAS_State.status, "RESOLVING",16)==0 or strncmp(own_TCAS_State.status, "RETURNING",16)==0){
        g2_set_font_size (virtual_dev, 20);
        g2_string(virtual_dev, 0, 60, own_TCAS_State.resolution);
    }
    
    g2_pen(virtual_dev, stuff_colour);
    
    sprintf(ID_str, "%lu", ownState.AC_ID);
    g2_set_font_size (virtual_dev, 20);
    g2_string(virtual_dev, 0, H-20, ID_str);
    
    Radar_set_centre(ownState);
    char pos_STR[32];
    double P_llh[3];
    xyz_to_llh(Centre_xyz, P_llh);
    sprintf(pos_STR, "ALT  %6.1f m", P_llh[2]);
    g2_string(virtual_dev, W-200, 20, pos_STR);
    sprintf(pos_STR, "VERT %6.1f m/s", Centre_V_enu[2]);
    g2_string(virtual_dev, W-200, 40, pos_STR);
    
    
    double heading = atan2(Centre_V_enu[0], Centre_V_enu[1])*180/pi;
    if(heading<0)
        heading+=360;
    sprintf(pos_STR, "HEAD %03d", (int)heading);
    g2_string(virtual_dev, W/2-35, H/2+R+10, pos_STR);
    
    for(unsigned int i=0; i<targetStates.size(); i++){
        
        if(ownState.AC_ID==0)
            continue;
        
        double xyz[3];
        xyz[0] = targetStates[i].x_pos;
        xyz[1] = targetStates[i].y_pos;
        xyz[2] = targetStates[i].z_pos;
        
        g2_pen(virtual_dev, get_colour(target_TCAS_States[i].status));
        
        sprintf(ID_str, "%lu", targetStates[i].AC_ID);
        Radar_draw_plane(xyz, ID_str);
        //g2_string(virtual_dev, 0, H-80, target_TCAS_States[i].status);
        
    }
    
    g2_flush(virtual_dev);
}
/*
int main(){
    Radar_initialize();
    double pos[3] = {6378137.0, 5e3, 3e3};
    double pos2[3] = {6378137.0, -2e3, 8e3};
    while(1){
        pos[1] -= 1e2;
        pos2[2] -= 1e2;
        Radar_draw_background();
        Radar_set_centre(pos);
        Radar_draw_plane(pos2);
        g2_flush(virtual_dev);
        usleep(50000);
    }
    return 0;
}*/
