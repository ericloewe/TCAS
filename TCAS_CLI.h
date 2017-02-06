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


TCAS_CLI: Definitions for the CLI

 */

#pragma once

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