#include "Radar.h"

int virtual_dev, physical_dev; 

bool centre_set = false;
double Centre_Latitude, Centre_Longitude;
double Centre_xyz[3];

int background_colour, stuff_colour;

void Radar_initialize(){
        
    physical_dev = g2_open_X11 (W, H);
    virtual_dev = g2_open_vd ();
    g2_clear (virtual_dev);
    g2_attach (virtual_dev, physical_dev);
    g2_set_auto_flush (virtual_dev, 0);
    
    background_colour = g2_ink(physical_dev, 0, 0, 0);
    stuff_colour      = g2_ink(physical_dev, 0, 0.75, 0); 
    
    g2_set_font_size (virtual_dev, 60);
    
    Radar_draw_background();
    g2_flush(virtual_dev);
}

void Radar_set_centre(double pos_xyz[3]){
    
    for(int i=0; i<3; i++){
        Centre_xyz[i] = pos_xyz[i];
    }
    double Centre_llh[3];
    xyz_to_llh(Centre_xyz, Centre_llh);
    Centre_Latitude = Centre_llh[0];
    Centre_Longitude = Centre_llh[1];
    
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

void Radar_draw_plane(double P_xyz[3]){
    
    if(!centre_set)
        _exit(1);
    
    double Dir_to_P[3];
    for(int i=0; i<3; i++)
        Dir_to_P[i] = P_xyz[i] - Centre_xyz[i];
    
    double Dir_to_P_ENU[3];
    xyz_to_enu(Dir_to_P, Centre_Latitude, Centre_Longitude, Dir_to_P_ENU);
    
    if( (Dir_to_P_ENU[0]*Dir_to_P_ENU[0] + Dir_to_P_ENU[1]*Dir_to_P_ENU[1]) > Range*Range)
        return;

    g2_pen(virtual_dev, stuff_colour);
    g2_filled_circle(virtual_dev, (double)W/2.0 + (double)R*Dir_to_P_ENU[0]/Range, (double)H/2.0 + (double)R*Dir_to_P_ENU[1]/Range, r);
}

void Radar_update(AC_state ownState, AC_state *targetStates){
    
    Radar_draw_background();
    
    double xyz[3] = {ownState.x_pos, ownState.y_pos, ownState.z_pos};
    Radar_set_centre(xyz);
    
    for(int i=0; i<TCAS_sim::NumOfTargets; i++){
        
        if(ownState.AC_ID==0)
            continue;
        
        xyz[0] = targetStates[i].x_pos;
        xyz[1] = targetStates[i].y_pos;
        xyz[2] = targetStates[i].z_pos;
        
        
        Radar_draw_plane(xyz);
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
