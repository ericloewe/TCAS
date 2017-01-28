
#include "Navigation.h"

//Coordinate conversions:

void xyz_to_llh(const double xyz[3], double llh[3]){
    
    double a = 6378137.0; //m
    double f = 1/298.257223563;
    
    llh[1] = atan2(xyz[1], xyz[0]); //longitude    
    
    //auxiliary variables:
    double P = sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1]);
    
    double sin_beta = xyz[2];
    double cos_beta = (1-f)*P;
    double norm_beta = sqrt(cos_beta*cos_beta + sin_beta*sin_beta);
    cos_beta /= norm_beta;
    sin_beta /= norm_beta;
    
    double e2 =  (1 - (1-f)*(1-f) );
    double ee2 = ( 1/((1-f)*(1-f)) -1);
    
    double sin_theta = xyz[2] + ee2*a*(1-f) * sin_beta*sin_beta*sin_beta; 
    double cos_theta = P      -  e2*a       * cos_beta*cos_beta*cos_beta;
    //double cos_theta = xyz[2] - xyz[2]*xyz[2]*xyz[2]*a*(1/(f-1)-(f-1));
    //double sin_theta = P - P*P*P*a*f*(f-1)*(f-2);
    double norm = sqrt(cos_theta*cos_theta + sin_theta*sin_theta);
    
    llh[0] = atan2( sin_theta, cos_theta ); //latitude
    
    cos_theta /= norm;
    sin_theta /= norm;
    
    llh[2] = P*cos_theta + xyz[2]*sin_theta - a*sqrt(1 + f*(f-2)*sin_theta*sin_theta); //altitude
    
}

void llh_to_xyz(const double llh[3], double xyz[3]){
    
    double a = 6378137.0; //m
    double f = 1/298.257223563;
    
    double RN = a/sqrt(1-f*(2-f)*sin(llh[0])*sin(llh[0]));
    
    xyz[0] = (RN+llh[2])*cos(llh[0])*cos(llh[1]);
    xyz[1] = (RN+llh[2])*cos(llh[0])*sin(llh[1]);
    xyz[2] = ((1-f)*(1-f)*RN + llh[2])*sin(llh[0]);
    
}

void aed_to_enu(const double aed[3], double enu[3]){
    enu[0] = aed[2]*cos(aed[1])*sin(aed[0]); //East
    enu[1] = aed[2]*cos(aed[1])*cos(aed[0]); //North
    enu[2] = aed[2]*sin(aed[1]); //Up
}

void enu_to_xyz(const double enu[3], const double latitude, const double longitude, double xyz[3]){
    xyz[0] = -enu[0]*sin(longitude)
             -enu[1]*cos(longitude)*sin(latitude)
             +enu[2]*cos(longitude)*cos(latitude);
             
    xyz[1] =  enu[0]*cos(longitude)
             -enu[1]*sin(longitude)*sin(latitude)
             +enu[2]*sin(longitude)*cos(latitude);
             
    xyz[2] =  enu[1]*cos(latitude)
             +enu[2]*sin(latitude);
    
}

void xyz_to_enu(const double xyz[3], const double latitude, const double longitude, double enu[3]){
    
    enu[0] =  -xyz[0]*sin(longitude)
              +xyz[1]*cos(longitude);
    
    enu[1] =  -xyz[0]*sin(latitude)*cos(longitude)
              -xyz[1]*sin(latitude)*sin(longitude)
              +xyz[2]*cos(latitude);
              
    enu[2] =   xyz[0]*cos(latitude)*cos(longitude)
              +xyz[1]*cos(latitude)*sin(longitude)
              +xyz[2]*sin(latitude);
    
}
