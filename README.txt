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


To compile, use make.

The following dependencies are required:
    libg2-dev
    libg20
    libgd2-xpm-dev
    libxext-dev
    x11-utils
    xfonts-75dpi
    xfonts-base
    xfonts-encoding
    xfonts-mathml
    xfonts-utils
    ttf-mscorefonts-installer

Your environment must provide a C++11-compatible environment. This software 
will not compile with previous C++ versions. It has been tested with the following 
versions:
C++ compiler: gcc (Ubuntu 6.2.0-5ubuntu12) 6.2.0 20161005
OS:           Ubuntu GNU/Linux Release 16.10 Yakkety Yak
g2:           Version 0.72

32-bit environments have not been tested but should work.
Little-endian (x86, x64) and Big-endian architectures (ARM, etc.) 
are not interoperable and will lead to garbled data. 



To run, use ./tcas "init filename". If no init file is provided, default 
configuration is assumed.

An example init file, exampleinit.txt, is provided with instructions on 
how to set one up.

A test setup with two instances running on one machine is available by using 
a pair of UDP ports. One instance sends to port A and receives on port B. The
second instance must send to port B and receive on port A.

