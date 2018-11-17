#ifndef CAR_CTRL_CLIENT_HPP
#define CAR_CTRL_CLIENT_HPP

#define GO_SERVICE_ID 0x0001
#define GO_INSTANCE_ID 0x0011

#define DIST_STEER_SERVICE_ID 0x0002 
#define DIST_STEER_INSTANCE_ID 0x0022
#define STEER_METHOD_ID 0x0222
#define DIST_EVENT_ID 0xE002
#define DIST_EVENTGROUP_ID 0xE022

#define MOTOR_SPEED_SERVICE_ID 0x0003
#define MOTOR_SPEED_INSTANCE_ID 0x0033
#define MOTOR_METHOD_ID 0x0333
#define SPEED_EVENT_ID 0xE003
#define SPEED_EVENTGROUP_ID 0xE033

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vsomeip/vsomeip.hpp>

class CarCTRLClient {
public:
    CarCTRLClient(uint32_t, uint32_t, uint32_t);
    bool init();
    void start();

    void on_state(vsomeip::state_type_e);
    void on_availability(vsomeip::service_t, vsomeip::instance_t, bool); //TODO set as private?
    void on_dist_eve(const std::shared_ptr<vsomeip::message>&);


    void run_st();
    void run_mo();
    std::thread req_st_thread_;
    std::thread req_mo_thread_;
private:
    bool is_init_;
    bool go_;
    uint32_t slaves_tot_;
    uint32_t slaves_disc_;
    std::mutex mu_init_;
    std::condition_variable cond_init_;
    void check_go_status();

    bool is_ava_st_;
    bool is_ava_mo_;

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<vsomeip::payload> payload_;
    std::shared_ptr<vsomeip::message> request_;

    uint32_t req_st_sleep_;
    uint32_t req_mo_sleep_;
    bool app_busy_;
    std::mutex mu_req_;
    std::condition_variable cond_req_;
};

#endif
