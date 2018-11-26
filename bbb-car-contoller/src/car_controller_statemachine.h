#ifndef __car_controller_statemachine__
#define __car_controller_statemachine__

#include <stdint.h>

#define TARGET_SPEED_STEP 1
#define TARGET_SPEED_STOP 0
#define TARGET_SPEED_MAX 100

#define DISTANCE_STOP 100   //cm

#define SPEED_OK 50 // just set for test currently 

#define DISTANCE_PRIO 0
#define DIRECTION_PRIO 0
#define SPEED_PRIO 0 // just set for test currrently

#define DIRECTION_LEFT -1
#define DIRECTION_STRAIGHT 0
#define DIRCTION_RIGHT 1

#define CAMERA_RUN 1
#define CAMERA_STOP 0
#define CAMERA_LEFT 3
#define CAMERA_RIGHT 4

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
    evRight
} evCameraEvent;

// structure of a sigle state
typedef struct{
    stCarState state;
    evCarEvent distance;
    evCarEvent speed;
    evCameraEvent camera;
    uint8_t targetSpeed;
    uint8_t direction; // -1 - left, 0 - straight, 1 - right
} stCarStatemachine;

// used for state transformation
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
void statemachineGetEvents();
void statemachineIteration();

// functions below are waitting for 
//  detailed library

/* int getFlag();
int* getDistance();
int getSpeed();
bool start();

void setMinDistance(int distance); // in cm
void setDistance(int distance, int prio);
void setDirection(int direction, int prio);
void setSpeed(int Speed, int prio); */

#endif