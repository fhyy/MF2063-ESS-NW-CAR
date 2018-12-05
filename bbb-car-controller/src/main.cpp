#include "ess_prototype.hpp"
#include "controller_statemachine.hpp"
#include <stdio.h>

/**
 * Check the status of the system before the statemachine is started
 */
bool bootup(ESSPrototype* prototype){
	if(!prototype->checkMotor()){
		printf("motor doesn't pass the check\n");
		return 0;
	}else if (!prototype->checkSpeedSensor()){
		printf("Speed Sensor doesn't pass the check\n");
		return 0;
	}else if (!prototype->checkSteering()){
		printf("Steering doesn't pass the check\n");
		return 0;
	}else if(!prototype->checkDistanceSensor()){
		printf("DistanceSensor doesn't pass the check\n");
		return 0;
	}

	/*bool bSetMinDi = prototype->setMinDistance(DISTANCE_STOP, DISTANCE_PRIO);
	if(!bSetMinDi){
		perror("Failed to set the minimum distance\n");
		return 0;		
	}*/
	return 1;
}



int main() {
	ESSPrototype prototype;

	sleep(2);
	bool bootFlag = bootup(&prototype);
	stCarStatemachine carStatemachine;
	bootFlag = true;
	statemachineInit(&carStatemachine);
	printState(&carStatemachine);
	if(bootFlag){
		printf("**********************************");
		printf("\nESS prototype car is online now!\n");
		printf("**********************************");
		while(1){
			statemachineIteration(&prototype);
			printState(&carStatemachine);
			
			usleep(500000);// 500ms
		}
	}

	return 0;
}
