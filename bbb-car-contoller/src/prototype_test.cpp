#include <iostream>
#include "ess_prototype.cpp"

int main() {

	ESSPrototype prototype;

    sleep(3);
    for (char i=0; true; i++) {
        prototype.setDirection(i, false);
        sleep(3);
    }
	return 0;
}
