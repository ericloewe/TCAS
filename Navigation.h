#pragma once

#include <cmath>
const double pi = 3.141592653589793;

void xyz_to_llh(const double xyz[3], double llh[3]);
void llh_to_xyz(const double llh[3], double xyz[3]);
void aed_to_enu(const double aed[3], double enu[3]);
void enu_to_xyz(const double enu[3], const double latitude, const double longitude, double xyz[3]);
void xyz_to_enu(const double xyz[3], const double latitude, const double longitude, double enu[3]);
