#pragma once

extern "C"{
#include <g2.h>
#include <g2_X11.h>
}
#include <unistd.h>
#include "Navigation.h"
#include "TCAS_defs.h"
#include "TCAS_sim.h"

//Can't these be variable?
const int W = 800;
const int H = 800;
const int R = 350;
const int r = 10;

const double Range = 35e3; //m

void Radar_set_centre(double pos_xyz[3]);

void Radar_initialize();
void Radar_draw_background();

void Radar_draw_plane(double P_xyz[3]);

void Radar_update(AC_state ownState, AC_state *targetStates);

