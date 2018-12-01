#include "ess_prototype.hpp"
#include "car_controller_statemachine.hpp"
#include <stdio.h>

/**
 * Check the status of the system before the statemachine is started
 */
bool bootup(ESSPrototype* prototype){
	if(!prototype->checkMotor()){
		perror("motor doesn't pass the check\n");
		return 0;
	}else if (!prototype->checkSpeedSensor()){
		perror("Speed Sensor doesn't pass the check\n");
		return 0;
	}else if (!prototype->checkSteering()){
		perror("Steering doesn't pass the check\n");
		return 0;
	}else if(!prototype->checkDistanceSensor()){
		perror("DistanceSensor doesn't pass the check\n");
		return 0;
	}

	bool bSetMinDi = prototype->setMinDistance(DISTANCE_STOP, DISTANCE_PRIO);
	if(!bSetMinDi){
		perror("Failed to set the minimum distance\n");
		return 0;		
	}
	return 1;
}



int main() {
	ESSPrototype prototype;

	sleep(2);
	bool bootFlag = bootup(&prototype);
	stCarStatemachine carStatemachine;
	
	if(bootFlag){
		printf("**********************************");
		printf("\nESS prototype car is online now!\n");
		printf("**********************************");
		while(1){
			statemachineInit(&carStatemachine);
			statemachineIteration(&prototype);
			printState(&carStatemachine);
			
			usleep(500000);// 500ms
		}
	}

	return 0;
}
