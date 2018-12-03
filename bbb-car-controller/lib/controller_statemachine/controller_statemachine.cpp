#include "controller_statemachine.hpp"
#include "ess_prototype.hpp"
#include <stdint.h>


stCarStatemachine *sm = 0x0;

void stopping(ESSPrototype* pty){
    sm->targetSpeed = TARGET_SPEED_STOP;
    sm->direction = DIRECTION_STRAIGHT;

    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed((char)(sm->targetSpeed), SPEED_PRIO);
}

void accelerating(ESSPrototype* pty){
    sm->targetSpeed += TARGET_SPEED_STEP;
    if(sm->targetSpeed > TARGET_SPEED_MAX){
        sm->targetSpeed = TARGET_SPEED_MAX;
    }
    sm->direction = DIRECTION_STRAIGHT;

    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed((char)(sm->targetSpeed), SPEED_PRIO);
}

void constantSpeed(ESSPrototype* pty){
    sm->direction = DIRECTION_STRAIGHT;

    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed((char)(sm->targetSpeed), SPEED_PRIO);
}

void retarding(ESSPrototype* pty){
    sm->targetSpeed -= TARGET_SPEED_STEP;
    if(sm->targetSpeed < TARGET_SPEED_STOP){
        sm->targetSpeed = TARGET_SPEED_STOP;
        sm->state = stateStandingStill;
    }
    sm->direction = DIRECTION_STRAIGHT;
    
    pty->setDirection(sm->direction, DIRECTION_PRIO);
    pty->setSpeed((char)(sm->targetSpeed), SPEED_PRIO);

}

void steeringLeft(ESSPrototype* pty){
    sm->direction = DIRECTION_LEFT;
    pty->setDirection(sm->direction, DIRECTION_PRIO);
}

void steeringRight(ESSPrototype* pty){
    sm->direction = DIRECTION_RIGHT;
    pty->setDirection(sm->direction, DIRECTION_PRIO);
}


void (*stateFunctionAry[6])(ESSPrototype*) = {
    stopping,
    accelerating,
    constantSpeed,
    retarding,
    steeringLeft,
    steeringRight
    };

//currently put the steering flag in the camera
static stStateTransfor stateMatrix[] = {
    //currentState  distanceFlag   speedFlag  cameraFlag  goStatus nextState
    {stateAny, evCarAny, evCarAny, evStop, evGoStatusAny, stateStandingStill},
    {stateAny,  evLow,  evCarAny, evCameraAny, evTrue,  stateStandingStill},
    {stateAny, evCarAny, evCarAny, evCameraAny, evFalse, stateStandingStill},

    
    {stateAny, evCarAny, evCarAny, evLeft, evTrue, stateSteeringLeft},
    {stateAny, eCarvAny, evCarAny, evRight, evTrue, stateSteeringRight},

    {stateSteeringLeft, evCarAny, evCarAny, evRun, evTrue, stateConstatntSpeed},
    {stateSteeringRight, evCarAny, evCarAny, evRun, evTrue, stateConstatntSpeed},

    {stateStandingStill, evHigh, evLow, evRun, evTrue, stateAccelerating},

    {stateAccelerating, evHigh, evLow, evRun, evTrue, stateAccelerating},
    {stateAccelerating, evHigh, evHigh, evRun, evTrue, stateRetarding},
    {stateAccelerating, evHigh, evOk, evRun, evTrue, stateConstantSpeed},
 
    {stateConstantSpeed, evHigh, evOk, evRun, evTrue, stateConstantSpeed},
    {stateConstantSpeed, evHigh, evHigh, evRun, evTrue, stateRetarding},
    {stateConstantSpeed, evHigh, evLow, evRun, evTrue, stateAccelerating},

    {stateRetarding, evHigh, evHigh, evRun, evTrue, stateRetarding},
    {stateRetarding, evHigh, evLow, evRun, evTrue, stateAccelerating},
    {stateRetarding, evHigh, evOk, evRun,  evTrue,  stateConstantSpeed},
};

void statemachineInit(stCarStatemachine* statemachine){
    sm = statemachine;
    sm->state = stateStandingStill;
    sm->speed = evLow;
    sm->distance = evHigh;
    sm->camera  =  evStop;
    sm->targetSpeed = TARGET_SPEED_STOP;
    sm->direction = DIRECTION_STRAIGHT;
    sm->goStatus = evFalse;
}

void statemachineGetEvents(ESSPrototype* pty){
    char speed, distance;
    bool goStatus;
    Flag camera;
    // read data from different places
    speed = pty->getSpeed();
    camera = pty->getFlag();
    distance = pty->getDistance();
    goStatus = pty->getGoStatus();
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
    if(camera.col == Flag::Red){
        sm->camera = evStop;
    }else if (camera.col == Flag::Green && camera.pos == Flag::Middle){
        sm->camera = evRun;
    }else if(camera.col == Flag::Green && camera.pos == Flag::Left){
        sm->camera = evLeft;
    }else if (camera.col == Flag::Green && camera.pos == Flag::Right){
        sm->camera = evRight;
    }else {
        // if weried msg is received, stop the car
        sm->camera = evStop;
    }
    // goStatus setup
    if(goStatus == TRUE)
        sm->goStatus = evTrue;
    else
        sm->goStatus = evFalse;
}

void statemachineIteration(ESSPrototype* pty){
    int i = 0;
    statemachineGetEvents(pty);
    for (i = 0; i < sizeof(stateMatrix) / sizeof(stateMatrix[0]);i++){
        if(sm->state == stateMatrix[i].currentState || stateMatrix[i].currentState==stateAny){
            if(sm->speed == stateMatrix[i].currentSpeedEvent || stateMatrix[i].currentSpeedEvent == evCarAny){
                if(sm->distance == stateMatrix[i].currentDistanceEvent || stateMatrix[i].currentDistanceEvent == evCarAny){
                    if(sm->camera == stateMatrix[i].currentCameraEvent || stateMatrix[i].currentCameraEvent == evCameraAny){
                        if(sm->goStatus == stateMatrix[i].currentGoStatus || sm->goStatus == evGoStatusAny)
                            sm->state = stateMatrix[i].nextState;
                            (*stateFunctionAry[sm->state])(pty);
                            break;
                    }
                }
            }
        }
    }
}

void printState(stCarStatemachine* statemachine){
    printf("\n state: %s, distance: %s, speed: %s, camera: %s, goStatus: %s, targetSpeed: %d\n", 
        stateText[statemachine->state],eventText[statemachine->distance],eventText[statemachine->speed],cameraText[statemachine->camera],goStatusText[statemachine->goStatus],statemachine->targetSpeed);
}