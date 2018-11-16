#ifndef DIST_STEER_SERVICE_HPP
#define DIST_STEER_SERVICE_HPP

#define GO_SERVICE_ID 0x0001
#define GO_INSTANCE_ID 0x0011

#define DIST_STEER_SERVICE_ID 0x0002 
#define DIST_STEER_INSTANCE_ID 0x0022
#define STEER_METHOD_ID 0x0222
#define DIST_EVENT_ID 0xE002
#define DIST_EVENTGROUP_ID 0xE022

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vsomeip/vsomeip.hpp>

class DistSteerService {
public:
    DistSteerService(uint32_t);
    bool init();
    void start();

    void on_state(vsomeip::state_type_e _state);
    void on_steer_req(const std::shared_ptr<vsomeip::message>&); // TODO arg name necessary?
    void on_go(vsomeip::service_t, vsomeip::instance_t, bool); // TODO handles availability event when GO server goes up

    void run_di();
    std::thread pub_di_thread_;
private:
    bool is_init_;
    bool go_;
    std::mutex mu_init_;
    std::condition_variable cond_init_;

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<vsomeip::payload> payload_;

    uint32_t pub_di_sleep_;
};


#endif
