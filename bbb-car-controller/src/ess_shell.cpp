#include <sstream>
#include <iostream>
#include "ess_prototype.hpp"

#define SETSPEED_CMD 1
#define SETSTEER_CMD 2
#define GETSPEED_CMD 3
#define GETDIST_CMD 4
#define GETCAMERA_CMD 5
#define CHECK_MOTOR 6
#define CHECK_STEER 7
#define CHECK_SPEEDOMETER 8
#define CHECK_DISTSENSORS 9
#define CHECK_CAMERA 10
#define QUIT 11

int getArgument();
string decodeFlag(Flag f);

using namespace std;

int main() {
    ESSPrototype proto;

    cout << "****************************************************" << std::endl;
    cout << "****************************************************" << std::endl;
    cout << "***                                              ***" << std::endl;
    cout << "***         #######   ###########   ###########  ***" << std::endl;
    cout << "***        #######   ##########    ##########    ***" << std::endl;
    cout << "***       ###         ###  ###      ###  ###     ***" << std::endl;
    cout << "***      #######       ###           ###         ***" << std::endl;
    cout << "***     #######         ###           ###        ***" << std::endl;
    cout << "***    ###          ###  ###      ###  ###       ***" << std::endl;
    cout << "***   #######      ##########    ##########      ***" << std::endl;
    cout << "***  #######      ##########    ##########       ***" << std::endl;
    cout << "***                                              ***" << std::endl;
    cout << "****************************************************" << std::endl;
    cout << "****************************************************" << std::endl;

    while(true) {
        cout << "Select command:" << std::endl;
        cout << "1. Set speed" << std::endl;
        cout << "2. Set steering" << std::endl;
        cout << "3. Get speed" << std::endl;
        cout << "4. Get distance" << std::endl;
        cout << "5. Get camera" << std::endl;
        cout << "6. Check motor" << std::endl;
        cout << "7. Check steering" << std::endl;
        cout << "8. Check speedometer" << std::endl;
        cout << "9. Check distance sensors" << std::endl;
        cout << "10. Check camera" << std::endl;
        cout << "11. Quit" << std::endl;

        int input = getArgument();

        switch (input) {
            case SETSPEED_CMD:
                proto.setSpeed((char) getArgument(), false);
                break;
            case SETSTEER_CMD:
                proto.setDirection((char) getArgument(), false);
                break;
            case GETSPEED_CMD:
                cout << "Current speed is " << (int) proto.getSpeed() << "cm/s." << std::endl;
                break;
            case GETDIST_CMD:
                cout << "Free distance ahead is " << (int) proto.getDistance() << "cm." << std::endl;
                break;
            case GETCAMERA_CMD:
                cout << decodeFlag(proto.getFlag()) << std::endl;
                break;
            case CHECK_MOTOR:
                cout << "Motor is " + string((proto.checkMotor() ? "" : "NOT "))
                     + "online." << std::endl;
                break;
            case CHECK_STEER:
                cout << "Steering is " + string((proto.checkSteering() ? "" : "NOT "))
                     + "online." << std::endl;
                break;
            case CHECK_SPEEDOMETER:
                cout << "Speedometer is " + string((proto.checkSpeedSensor() ? "" : "NOT "))
                     + "online." << std::endl;
                break;
            case CHECK_DISTSENSORS:
                cout << "Distance sensors are " + string((proto.checkDistanceSensor() ? "" : "NOT "))
                     + "online." << std::endl;
                break;
            case CHECK_CAMERA:
                cout << "Camera is " + string((proto.checkCameraSensor() ? "" : "NOT "))
                     + "online." << std::endl;
                break;
            case QUIT:
                goto endloop;
                break;
            default:
                break;
        }
    }
endloop:
    // TODO stop proto?
    return 0;
}

int getArgument() {
    string raw;
    stringstream conv;
    int input = 0;

    getline(cin, raw);
    conv << raw;
    conv >> input;

    // TODO error handling

    return input;
}

string decodeFlag(Flag f) {
    string msg("");

    switch (f.col) {
        case Flag::Red : msg += "Flag is red and ";
                         break;
        case Flag::Green : msg += "Flag is green and ";
                           break;
        case Flag::Yellow : msg += "Flag is yellow and";
                            break;
        default : msg += "Flag has no color and is ";
                  break;
    }
    switch (f.pos) {
        case Flag::Left : msg += "to the left.";
                          break;
        case Flag::Right : msg += "to the right.";
                           break;
        case Flag::Middle : msg += "in the middle.";
                            break;
        default : msg += "nowhere to be found.";
                  break;
    }
    return msg;
}
