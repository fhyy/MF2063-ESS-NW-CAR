#include "car_ctrl_client.hpp"

int main(int argc, char** argv) {
    CarCTRLClient ccc(2000, 2500);

    if(ccc.init()) {
        ccc.start();
        ccc.req_st_thread_.join();
        ccc.req_mo_thread_.join();
        return 0;
    }
    else
        return 1;
}
