chi#ifndef __car_controller_statemachine__
#define __car_controller_statemachine__

#include <stdint.h>
#include <ess_prototype.hpp>

#define TARGET_SPEED_STEP 1
#define TARGET_SPEED_STOP 0
#define TARGET_SPEED_MAX 100

#define DISTANCE_STOP 100   //cm

#define SPEED_OK 50 // just set for test currently 

#define DISTANCE_PRIO 0
#define DIRECTION_PRIO 0
#define SPEED_PRIO 0 // just set for test currrently

#define DIRECTION_LEFT 1
#define DIRECTION_STRAIGHT 0
#define DIRCTION_RIGHT 2

#define CAMERA_RUN 1
#define CAMERA_STOP 0
#define CAMERA_LEFT 2
#define CAMERA_RIGHT 3


// Enumeraiion of states
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

// Enumeration of events of speed and distance
typedef enum
{
    evHigh=0,
    evLow,
    evOk,
    evAny
} evCarEvent;
// Enumeraiion of events of camera
typedef enum
{
    evStop=0,
    evRun,
    evLeft,
    evRight,
    evAny
} evCameraEvent;

typedef enum
{
    evTrue=0,
    evFalse,
    evAny
}evGoStatusEvent;

// structure of a sigle state
typedef struct{
    stCarState state;
    evCarEvent distance;
    evCarEvent speed;
    evCameraEvent camera;
    evGoStatusEvent goStatus;
    uint8_t targetSpeed;
    uint8_t direction; // 1 - left, 0 - straight, 2 - right
} stCarStatemachine;

// used for state transformation
typedef struct{
    stCarState currentState;
    evCarEvent currentDistanceEvent;
    evCarEvent currentSpeedEvent;
    evCameraEvent currentCameraEvent;
    evGoStatusEvent currentGoStatus;
    stCarState nextState;
} stStateTransfor;
typedef struct{
    uint32_t speed;
    uint32_t distance;
    uint32_t camera;
} stControllerInput;

static char* stateText[]={
    "STANDING_STILL",
    "ACCELERATING",
    "CONSTANT_SPEED",
    "RETARDING",
    "STEERING_LEFT",
    "STEERINF_RIGHT"
};

static char* eventText[]={
    "HIGH",
    "LOW",
    "OK"
};

static char* cameraText[]={
    "STOP",
    "RUN",
    "LEFT",
    "RIGHT"
};

static char* goStatusText[]={
    "TRUE",
    "FALSE"
};



void statemachineInit(stCarStatemachine *statemachine);
void statemachineGetEvents(ESSPrototype* pty);
void statemachineIteration(ESSPrototype* pty);
void printState(stCarStatemachine* statemachine);


#endif