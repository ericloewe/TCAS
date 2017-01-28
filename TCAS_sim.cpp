#include "TCAS_sim.h"

bool TCAS_sim::UpdateOwnState(AC_state newState)
{
    //newState must be valid and ownState AC_ID can't change unless it is undefined
    if(newState.AC_ID==0 || (newState.AC_ID!=ownState.AC_ID && ownState.AC_ID!=0))
        return false;
        
    //we can't be our targets
    for(int i=0; i<NumOfTargets; i++)
    {
        if(newState.AC_ID == targetStates[i].AC_ID)
        {
            return false;
        }
    }
    
    ownState = newState;
    return true;
}

bool TCAS_sim::UpdateTargetState(AC_state newState)
{
    if(newState.AC_ID == 0)
        return false;
    
    //we can't be our targets
    if(newState.AC_ID == ownState.AC_ID)
        return false;
    
    //find target somewhere
    for(int i=0; i<NumOfTargets; i++)
    {
        if(newState.AC_ID == targetStates[i].AC_ID)
        {
            targetStates[i] = newState;
            return true;
        }
    }
    
    //Aircraft not in targets. Insert in first available.
    for(int i=0; i<NumOfTargets; i++)
    {
        if(targetStates[i].AC_ID == 0)
        {
            targetStates[i] = newState;
            return true;
        }
    }
    
    return false;
}
