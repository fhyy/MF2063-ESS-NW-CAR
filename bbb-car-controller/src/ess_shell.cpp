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

    cout << "****************************************************" << endl;
    cout << "****************************************************" << endl;
    cout << "***                                              ***" << endl;
    cout << "***         #######   ###########   ###########  ***" << endl;
    cout << "***        #######   ##########    ##########    ***" << endl;
    cout << "***       ###         ###  ###      ###  ###     ***" << endl;
    cout << "***      #######       ###           ###         ***" << endl;
    cout << "***     #######         ###           ###        ***" << endl;
    cout << "***    ###          ###  ###      ###  ###       ***" << endl;
    cout << "***   #######      ##########    ##########      ***" << endl;
    cout << "***  #######      ##########    ##########       ***" << endl;
    cout << "***                                              ***" << endl;
    cout << "****************************************************" << endl;
    cout << "****************************************************" << endl;

    while(true) {
        cout << "==================================" << endl;
        cout << "Select command:" << endl;
        cout << "1. Set speed" << endl;
        cout << "2. Set steering" << endl;
        cout << "3. Get speed" << endl;
        cout << "4. Get distance" << endl;
        cout << "5. Get camera" << endl;
        cout << "6. Check motor" << endl;
        cout << "7. Check steering" << endl;
        cout << "8. Check speedometer" << endl;
        cout << "9. Check distance sensors" << endl;
        cout << "10. Check camera" << endl;
        cout << "11. Quit" << endl;

        int input = getArgument();

        switch (input) {
            case SETSPEED_CMD:
                cout << "Enter desired speed (0-125): ";
                proto.setSpeed((char) getArgument(), false);
                cout << endl;
                break;
            case SETSTEER_CMD:
                cout << "Enter desired angle (0-100): ";
                proto.setDirection((char) getArgument(), false);
                cout << endl;
                break;
            case GETSPEED_CMD:
                cout << "> Current speed is " << (int) proto.getSpeed() << " cm/s." << endl;
                break;
            case GETDIST_CMD:
                cout << "> Free distance ahead is " << (int) proto.getDistance() << " cm." << endl;
                break;
            case GETCAMERA_CMD:
                cout << decodeFlag(proto.getFlag()) << endl;
                break;
            case CHECK_MOTOR:
                cout << "> Motor is " + string((proto.checkMotor() ? "" : "NOT "))
                     + "online." << endl;
                break;
            case CHECK_STEER:
                cout << "> Steering is " + string((proto.checkSteering() ? "" : "NOT "))
                     + "online." << endl;
                break;
            case CHECK_SPEEDOMETER:
                cout << "> Speedometer is " + string((proto.checkSpeedSensor() ? "" : "NOT "))
                     + "online." << endl;
                break;
            case CHECK_DISTSENSORS:
                cout << "> Distance sensors are " + string((proto.checkDistanceSensor() ? "" : "NOT "))
                     + "online." << endl;
                break;
            case CHECK_CAMERA:
                cout << "> Camera is " + string((proto.checkCameraSensor() ? "" : "NOT "))
                     + "online." << endl;
                break;
            case QUIT:
                goto endloop;
                break;
            default:
                break;
        }

    cout << "==================================" << endl;
    }
endloop:
    // TODO stop proto?
    cout << "Quitting ess_shell..." << endl;
    cout << "==================================" << endl;
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
    string msg("> ");

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
