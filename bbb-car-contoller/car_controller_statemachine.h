#ifndef __car_controller_statemachine__
#define __car_controller_statemachine__

#include <stdint.h>

#define STATEMACHINE_THROTTLE_STEP 1
#define STATEMACHINE_THROTTLE_STOP 90
#define STATEMACHINE_THROTTLE_MAX 179

#define DISTANCE_STOP 100   //cm

#define SPEED_OK 50 // just set for test currently 


typedef enum
{
    stateStandingStill=0,
    stateAccelerating,
    stateConstantSpeed,
    stateRetarding,
    stateSteering,
    stateAny
} stCarState;

typedef enum
{
    evHigh=0,
    evLow,
    evOk,
    evAny
} evCarEvent;

typedef enum
{
    evStop=0,
    evRun,
    evLeft,
    evRight
} evCameraEvent;

typedef struct{
    stCarState state;
    evCarEvent distance;
    evCarEvent speed;
    evCameraEvent camera;
    uint32_t throttle;
} stCarStatemachine;

typedef struct{
    stCarState currentState;
    evCarEvent currentDistanceEvent;
    evCarEvent currentSpeedEvent;
    evCameraEvent currentCameraEvent;
    stCarState nextState;
} stStateTransfor;
typedef struct{
    uint32_t speed;
    uint32_t distance;
    uint32_t camera;
} stControllerInput;

void statemachineInit(stCarStatemachine *statemachine);
void statemachineGetEvents(stControllerInput *input);
void statemachineIteration(stControllerInput *input);
#endif