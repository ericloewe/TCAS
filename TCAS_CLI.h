#include <iostream>

void printStatusDisp(long long hex, 
                    double lat, double lon, double alt, 
                    double hdg, double TAS, double Vup);
void printStatusDisp(long long hex, 
                    double lat, double lon, double alt, 
                    double hdg, double TAS, double Vup,
                    std::string status, bool CRCpass, int timeout);
void printStatusHeader();
void printStatusHeaderLong();

void convertData(AC_state state,
                 double& lat, double& lon, double& altitude,
                 double& HDG, double& TAS, double& Vup);