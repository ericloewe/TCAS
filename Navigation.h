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


Navigation: Definitions for coordinate transformations

 */

#pragma once

#include <cmath>
const double pi = 3.141592653589793;

void xyz_to_llh(const double xyz[3], double llh[3]);
void llh_to_xyz(const double llh[3], double xyz[3]);
void aed_to_enu(const double aed[3], double enu[3]);
void enu_to_xyz(const double enu[3], const double latitude, const double longitude, double xyz[3]);
void xyz_to_enu(const double xyz[3], const double latitude, const double longitude, double enu[3]);
