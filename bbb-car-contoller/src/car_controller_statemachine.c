#include "car_controller_statemachine.h"
#include <stdint.h>

stCarStatemachine *sm = 0x0;

void stopping(){
    sm->targetSpeed = TARGET_SPEED_STOP;
    sm->direction = DIRECTION_STRAIGHT;

    setDirection(sm->direction, DIRECTION_PRIO);
    setSpeed(sm->targetSpeed, SPEED_PRIO);
}

void accelerating(){
    sm->targetSpeed += TARGET_SPEED_STEP;
    if(sm->targetSpeed > TARGET_SPEED_MAX){
        sm->targetSpeed = TARGET_SPEED_MAX;
    }
    sm->direction = DIRECTION_STRAIGHT;

    setDirection(sm->direction, DIRECTION_PRIO);
    setSpeed(sm->targetSpeed, SPEED_PRIO);
}

void constantSpeed(){
    sm->direction = DIRECTION_STRAIGHT;

    setDirection(sm->direction, DIRECTION_PRIO);
    setSpeed(sm->targetSpeed, SPEED_PRIO);
}

void retarding(){
    sm->targetSpeed -= TARGET_SPEED_STEP;
    if(sm->targetSpeed < TARGET_SPEED_STOP){
        sm->targetSpeed = TARGET_SPEED_STOP;
        sm->state = stateStandingStill;
    }
    sm->direction = DIRECTION_STRAIGHT;
    
    setDirection(sm->direction, DIRECTION_PRIO);
    setSpeed(sm->targetSpeed, SPEED_PRIO);

}

void steeringLeft(){
    sm->direction = DIRECTION_STRAIGHT;
    setDirection(sm->direction, DIRECTION_PRIO);
}

void steeringRight(){
    sm->direction = DIRECTION_STRAIGHT;
    setDirection(sm->direction, DIRECTION_PRIO);
}

typedef void (*func)(void);
static func stateFunctionAry[] = {
    &stopping,
    &accelerating,
    &constantSpeed,
    &retarding,
    &steeringLeft,
    &steeringRight
    };

//currently put the steering flag in the camera
static stStateTransfor stateMatrix[] = {
    //currentState  distanceFlag   speedFlag  cameraFlag nextState
    {stateAny, evAny, evAny, evStop, stateStandingStill},
    {stateAny,  evLow,  evAny, evRun,   stateStandingStill},
    
    {stateAny, evAny, evAny, evLeft, stateSteeringLeft},
    {stateAny, evAny, evAny, evRight, stateSteeringRight},

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
    sm = statemachine;
    statemachine->state = stateStandingStill;
    statemachine->speed = evLow;
    statemachine->distance = evHigh;
    statemachine->camera = evStop;
    statemachine->targetSpeed = TARGET_SPEED_STOP;
    statemachine->direction = DIRECTION_STRAIGHT;
}
// call self-defined library functions to get current car events
void statemachineGetEvents(){
    int speed, cameraFlag, averDistance;
    int *p;

    // read data from different places
    speed = getSpeed();
    cameraFlag = getFlag();
    p = getDistance();
    averDistance = (*p + *(p + 1) + *(p + 2)) / 3;

    // speed setup
   if(speed == SPEED_OK){
        sm->speed = evOk;
    }else if(speed > SPEED_OK){
        sm->speed = evHigh;
    }else{
        sm->speed = evLow;
    } 
    // distance setup
    if(averDistance > DISTANCE_STOP){
        sm->distance = evHigh;
    }else{
        sm->distance = evLow;
    }
    // camera setup, don't have detailed definitions of flags now
    if(cameraFlag == 1){
        sm->camera = evRun;
    }else{
        sm->camera = evStop;
    }
}

void statemachineIteration(){
    int i = 0;
    statemachineGetEvents();
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