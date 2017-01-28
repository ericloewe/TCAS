#include <iostream>
#include "TCAS_comms.h"

int const NofmemBuffers = 3;

int main(void)
{
    std::cout << "Allocating " << NofmemBuffers << " memBuffers" << std::endl;
    
    memBuffer memBufList[NofmemBuffers];
    
    int memBufSize = 4096;
    
    for (int i = 0; i < NofmemBuffers; i++)
    {
        memBufList[i].init(memBufSize);
    }
    
    std::cout << "Press enter to continue" << std::endl;
    std::cin.get();
}

