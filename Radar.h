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


Radar: Definitions for the GUI

 */


#pragma once

extern "C"{
#include <g2.h>
#include <g2_X11.h>
}
#include <vector>
#include <unistd.h>
#include <string.h>

#include "Navigation.h"
#include "TCAS_defs.h"
#include "TCAS_sim.h"

//Can't these be variable?
const int W = 800;
const int H = 800;
const int R = 350;
const int r = 7;

const double Range = 35e3; //m

void Radar_set_centre(double pos_xyz[3]);

void Radar_initialize();
void Radar_draw_background();

void Radar_draw_plane(double P_xyz[3]);

void Radar_update(AC_state ownState, TCAS_state own_TCAS_State, std::vector<AC_state> targetStates, std::vector<TCAS_state> target_TCAS_States);

