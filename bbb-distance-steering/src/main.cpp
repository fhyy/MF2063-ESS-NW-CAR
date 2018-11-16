#include "dist_steer_service.hpp"

int main(int argc, char** argv) {

    //TODO process user arguments

    DistSteerService dss;
    
    if (dss.init()) {
        dss.start();
        return 0;
    }
    else
        return 1;
}
