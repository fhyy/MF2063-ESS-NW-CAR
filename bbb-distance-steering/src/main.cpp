#include "dist_steer_service.hpp"

int main(int argc, char** argv) {

    //TODO process user arguments

    DistSteerService dss(7000);
    
    if (dss.init()) {
        dss.start();
        dss.pub_di_thread_.join();
        return 0;
    }
    else
        return 1;
}
