#ifndef __car_controller_statemachine__
#define __car_controller_statemachine__

#include <iostream>
#include <stdint.h>
#include "ess_prototype.hpp"

#define TARGET_SPEED_STEP 5
#define TARGET_SPEED_STOP 0
#define TARGET_SPEED_MAX 100 // cm/s

#define DISTANCE_STOP 50   //cm

#define SPEED_OK 50 // cm/s, just set for test currently 

#define DISTANCE_PRIO 0
#define DIRECTION_PRIO 0
#define SPEED_PRIO 0 // just set for test currrently

#define DIRECTION_STRAIGHT 50
#define DIRECTION_LEFT 0
#define DIRECTION_RIGHT 100


typedef enum
{
    stateStandingStill=0,
    stateAccelerating,
    stateConstantSpeed,
    stateRetarding,
    stateSteeringLeft,
    stateSteeringRight,
    stateAny
} stCarState;

/**
 * Enumeration of events of speed and distance
 */
typedef enum
{
    evHigh=0,
    evLow,
    evOk,
    evCarAny
} evCarEvent;

/**
 * Enumeraiion of events of camera
 */
typedef enum
{
    evStop=0,
    evRun,
    evLeft,
    evRight,
    evCameraAny
} evCameraEvent;

/**
 * Enumeraiion of events of go status
 */
typedef enum
{
    evTrue=0,
    evFalse,
    evGoStatusAny
}evGoStatusEvent;

/**
 * Structure of a sigle state
 */

typedef struct{
    stCarState state;
    evCarEvent distance;
    evCarEvent speed;
    evCameraEvent camera;
    evGoStatusEvent goStatus;
    int targetSpeed;
    uint8_t direction; // 1 - left, 0 - straight, 2 - right
} stCarStatemachine;

/**
 * Used for state transformation matrix
 */
typedef struct{
    stCarState currentState;
    evCarEvent currentDistanceEvent;
    evCarEvent currentSpeedEvent;
    evCameraEvent currentCameraEvent;
    evGoStatusEvent currentGoStatus;
    stCarState nextState;
} stStateTransfor;

/**
 * Only used in the test program,
 */
typedef struct{
    uint32_t speed;
    uint32_t distance;
    uint32_t camera;
} stControllerInput;

const std::string stateText[]={
    "STANDING_STILL",
    "ACCELERATING",
    "CONSTANT_SPEED",
    "RETARDING",
    "STEERING_LEFT",
    "STEERINF_RIGHT"
};

const std::string eventText[]={
    "HIGH",
    "LOW",
    "OK"
};

const std::string cameraText[]={
    "STOP",
    "RUN",
    "LEFT",
    "RIGHT"
};

const std::string goStatusText[]={
    "TRUE",
    "FALSE"
};


/**
 * Initial the statemachine
 * 
 */
void statemachineInit(stCarStatemachine *statemachine);

/**
 * Grab data from shared memory,
 * set flags for input flags of statemachine
 */
void statemachineGetEvents(ESSPrototype* pty);

/**
 * Search right entry in stateMatrix,
 * call corresponding function 
 */
void statemachineIteration(ESSPrototype* pty);

/**
 * Print current statemachine car_controller_statemachine,
 * used for test
 */
void printState(stCarStatemachine* statemachine);

bool doctor(ESSPrototype *prototype);

#endif
