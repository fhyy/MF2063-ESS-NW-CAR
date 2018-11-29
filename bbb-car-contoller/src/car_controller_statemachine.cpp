#include "car_controller_statemachine.h"
#include <stdint.h>

stCarStatemachine *sm = 0x0;

void stopping(ESSPrototype* pty){
    sm->targetSpeed = TARGET_SPEED_STOP;
    sm->direction = DIRECTION_STRAIGHT;

    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed(sm->targetSpeed, SPEED_PRIO);
}

void accelerating(ESSPrototype* pty){
    sm->targetSpeed += TARGET_SPEED_STEP;
    if(sm->targetSpeed > TARGET_SPEED_MAX){
        sm->targetSpeed = TARGET_SPEED_MAX;
    }
    sm->direction = DIRECTION_STRAIGHT;

    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed(sm->targetSpeed, SPEED_PRIO);
}

void constantSpeed(ESSPrototype* pty){
    sm->direction = DIRECTION_STRAIGHT;

    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed(sm->targetSpeed, SPEED_PRIO);
}

void retarding(ESSPrototype* pty){
    sm->targetSpeed -= TARGET_SPEED_STEP;
    if(sm->targetSpeed < TARGET_SPEED_STOP){
        sm->targetSpeed = TARGET_SPEED_STOP;
        sm->state = stateStandingStill;
    }
    sm->direction = DIRECTION_STRAIGHT;
    
    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed(sm->targetSpeed, SPEED_PRIO);

}

void steeringLeft(ESSPrototype* pty){
    sm->direction = DIRECTION_LEFT;
    pty->setDirection(sm->direction, DIRECTION_PRIO);
}

void steeringRight(ESSPrototype pty){
    sm->direction = DIRECTION_RIGHT;
    pty->setDirection(sm->direction, DIRECTION_PRIO);
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

    {stateSteeringLeft, evAny, evAny, evAny, stateConstatntSpeed},
    {stateSteeringRight, evAny, evAny, evAny, stateConstatntSpeed},

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
    statemah*ine->camera  =  evStop;
    statemachine->targetSpeed = TARGET_SPEED_STOP;
    statemachine->direction = DIRECTION_STRAIGHT;
}
// call sel
void statemachineGetEvents(ESSPrototype* pty){
    char speed, camera, distance;
    

    // read data from different places
    speed = pty->getSpeed();
    camera = pty->getFlag();
    distance = pty->getDistance();
    
    // speed setup
   if(speed == SPEED_OK){
        sm->speed = evOk;
    }else if(speed > SPEED_OK){
        sm->speed = evHigh;
    }else{
        sm->speed = evLow;
    } 
    // distance setup
    if(distance > DISTANCE_STOP){
        sm->distance = evHigh;
    }else{
        sm->distance = evLow;
    }
    // camera setup
    if(camera == CAMERA_STOP)
        sm->camera = evStop;
    }else if (camera == CAMERA_RUN){
        sm->camera = evRun;
    }else if(camera == CAMERA_LEFT){
        sm->camera = evLeft;
    }else if (camera == CAMERA_RIGHT){
        sm->camera = evRight;
    }else {
        // if wired msg is received, stop the car
        sm->camera = evStop;
    }
}

void statemachineIteration(ESSPrototype* pty){
    int i = 0;
    statemachineGetEvents(pty);
    for (i = 0; i < sizeof(stateMatrix) / sizeof(stateMatrix[0]);i++){
        if(sm->state == stateMatrix[i].currentState || stateMatrix[i].currentState==stateAny){
            if(sm->speed == stateMatrix[i].currentSpeedEvent || stateMatrix[i].currentSpeedEvent == evAny){
                if(sm->distance == stateMatrix[i].currentDistanceEvent || stateMatrix[i].currentDistanceEvent == evAny){
                    if(sm->camera == stateMatrix[i].currentCameraEvent ){
                        sm->state = stateMatrix[i].nextState;
                        (stateFunctionAry[sm->state])(pty);
                        break;
                    }
                }
            }
        }
    }
}

void printState(stCarStatemachine* statemachine){
    printf("\n state: %s, distance: %s, speed: %s, camera: %s\n, targetSpeed: %d\n", 
        stateText[statemachine->state],eventText[statemachine->distance],eventText[statemachine->speed],cameraText[statemachine->camera],statemachine->targetSpeed);
}