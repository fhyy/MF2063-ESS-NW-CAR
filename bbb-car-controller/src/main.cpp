#include "ess_prototype.hpp"
#include "controller_statemachine.hpp"
#include <stdio.h>


	



int main() {
	ESSPrototype prototype;

	sleep(2);
	bool bootFlag = doctor(&prototype);
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