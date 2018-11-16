#ifndef DIST_STEER_SERVICE_HPP
#define DIST_STEER_SERVICE_HPP

#define DIST_STEER_SERVICE_ID 0x0002 
#define DIST_STEER_INSTANCE_ID 0x0022
#define STEER_METHOD_ID 0x0222
#define DIST_EVENT_ID
#define DIST_EVENTGROUP_ID

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vsomeip/vsomeip.hpp>

class DistSteerService {
public:
    DistSteerService(); //TODO maybe take some arguments?
    bool init();
    void start();
    void on_state(vsomeip::state_type_e _state);
    void on_steer_req(const std::shared_ptr<vsomeip::message> &_msg);
private:
    void run_notify();

    std::shared_ptr<vsomeip::application> app_;
    uint32_t notify_period_;
    std::thread notify_thread_;
    std::mutex mu_init_;
    std::condition_variable cond_init_;
    bool is_init_;
};


#endif
