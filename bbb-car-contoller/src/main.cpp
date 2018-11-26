#include "ess_prototype.hpp"
#include "car_controller_statemachine.h"
#include <stdio.h>

// check the status of the system before we start
bool bootup(ESSPrototype prototype){
	if(!prototype.checkMotor()){
		perror("motor doesn't pass the check\n");
		return 0;
	}else if (!prototype.checkSpeedSensor()){
		perror("Speed Sensor doesn't pass the check\n");
		return 0;
	}else if (!prototype.checkSteering()){
		perror("Steering doesn't pass the check\n");
		return 0;
	}else if(!prototype.checkDistanceSensor()){
		perror("DistanceSensor doesn't pass the check\n");
		return 0;
	}

	bool bSetMinDi = prototype.setMinDistance(DISTANCE_STOP, DISTANCE_PRIO);
	if(!bSetMinDi){
		perror("Failed to set the minimum distance\n");
		return 0;		
	}
	return 1;
}

int main() {
	ESSPrototype prototype;
	bool bootFlag = bootup(prototype);
	stCarStatemachine sm;
	
	if(bootFlag){
		printf("\nESS prototype car is online now!\n");
		while(1){
			statemachineInit(&sm);
			statemachineIteration();
		}
	}

	return 0;
}
