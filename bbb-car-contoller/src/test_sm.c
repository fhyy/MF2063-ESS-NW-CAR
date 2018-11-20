#include <stdio.h>
#include <stdint.h>
#include "car_controller_statemachine.h"


static char* stateText[] = {
    "standingStill",
    "accelerating",
    "constantSpeed",
    "retarding"
};

static char* eventText[] = {
    "evHigh",
    "evLow",
    "evOK"
};
static char* eventText2[] = {
    "evStop", 
    "evRun"
};

static inline void statemachinePrint(stCarStatemachine* sm){
    printf("{state:%s, distance:%s, speed:%s, camera:%s, throttle:%d}\n", stateText[sm->state], eventText[sm->distance], eventText[sm->speed], eventText2[sm->camera],sm->throttle);
    printf("****************************\n");
}

int main(){
    stCarStatemachine sm;
    stControllerInput input;
    printf("****************************\n");
    printf("Car controller is online now\n");
    printf("****************************\n");
    
    statemachineInit(&sm);    
    statemachinePrint(&sm);

    while(1){
        printf("input current speed: (cm/s): ");
        scanf("%u", &input.speed);
        printf("input current distance (cm): ");
        scanf("%u",&input.distance);
        printf("input current camera signal (1 for run, 0 for stop): ");
        scanf("%u",&input.camera);
        
        statemachineIteration(&input);
        statemachinePrint(&sm);
    }
    return 0;

}