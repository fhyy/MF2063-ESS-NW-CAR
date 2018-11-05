#include "car_controller_statemachine.h"
#include <stdint.h>

stCarStatemachine *sm = 0x0;

void stopping(){
    sm->throttle = STATEMACHINE_THROTTLE_STOP;
}

void accelerating(){
    sm->throttle += STATEMACHINE_THROTTLE_STEP;
    if(sm->throttle > STATEMACHINE_THROTTLE_MAX){
        sm->throttle = STATEMACHINE_THROTTLE_MAX;
    }
}

void constantSpeed(){}

void retarding(){
    sm->throttle -= STATEMACHINE_THROTTLE_STEP;
    if(sm->throttle < STATEMACHINE_THROTTLE_STOP){
        sm->throttle = STATEMACHINE_THROTTLE_STOP;
        sm->state = stateStandingStill;
    }
}
typedef void (*func)(void);
static func stateFunctionAry[] = {
    &stopping,
    &accelerating,
    &constantSpeed,
    &retarding
    };

//currently put the steering flag in the camera,
//  may put it in distance node later
static stStateTransfor stateMatrix[] = {
    //currentState  distanceFlag   speedFlag  nextState
    {stateAny, evAny, evAny, evStop, stateStandingStill},
    {stateAny,  evLow,  evAny, evRun,   stateStandingStill},
    

    {stateStandingStill, evHigh, evLow, evRun, stateAccelerating},

    {stateAccelerating, evHigh, evLow, evRun, stateAccelerating},
    {stateAccelerating, evHigh, evHigh, evRun, stateRetarding},
    {stateAccelerating, evHigh, evOk, evRun, stateConstantSpeed},

 
    {stateConstantSpeed, evHigh, evOk, evRun, stateConstantSpeed},
    {stateConstantSpeed, evHigh, evHigh, evRun, stateRetarding},
    {stateConstantSpeed, evHigh, evLow, evRun, stateAccelerating},

    {stateRetarding, evHigh, evHigh, evRun, stateRetarding},
    {stateRetarding, evHigh, evLow, evRun, stateAccelerating},
    {stateConstantSpeed, evHigh, evOk, evRun,    stateConstantSpeed},
};

void statemachineInit(stCarStatemachine* statemachine){
    //sm = statemachine;
    statemachine->state = stateStandingStill;
    statemachine->speed = evLow;
    statemachine->distance = evHigh;
    statemachine->camera = evStop;
    statemachine->throttle = 90;
}

void statemachineGetEvents(stControllerInput* input){
    // speed setup
    if(input->speed == SPEED_OK){
        sm->speed = evOk;
    }else if(input->speed > SPEED_OK){
        sm->speed = evHigh;
    }else{
        sm->speed = evLow;
    }
    // distance setup
    if(input->distance > DISTANCE_STOP){
        sm->distance = evHigh;
    }else{
        sm->distance = evLow;
    }
    // camera setup
    if(input->camera == 1){
        sm->camera = evRun;
    }else{
        sm->camera = evStop;
    }
}

void statemachineIteration(stControllerInput* input){
    int i = 0;
    statemachineGetEvents(input);
    for (i = 0; i < sizeof(stateMatrix) / sizeof(stateMatrix[0]);i++){
        if(sm->state == stateMatrix[i].currentState || stateMatrix[i].currentState==stateAny){
            if(sm->speed == stateMatrix[i].currentSpeedEvent || stateMatrix[i].currentSpeedEvent == evAny){
                if(sm->distance == stateMatrix[i].currentDistanceEvent || stateMatrix[i].currentDistanceEvent == evAny){
                    if(sm->camera == stateMatrix[i].currentCameraEvent ){
                        sm->state = stateMatrix[i].nextState;
                        (stateFunctionAry[sm->state])();
                        break;
                    }
                }
            }
        }
    }
}