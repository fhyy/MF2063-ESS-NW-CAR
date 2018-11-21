#ifndef MOTOR_SPEED_SERVICE_HPP
#define MOTOR_SPEED_SERVICE_HPP

#define GO_SERVICE_ID 0x0001
#define GO_INSTANCE_ID 0x0001

#define SHUTDOWN_METHOD_ID 0x0666

#define DIST_SERVICE_ID 0x0022 
#define DIST_INSTANCE_ID 0x0022
#define DIST_EVENT_ID 0xE022
#define DIST_EVENTGROUP_ID 0xE022

#define MOTOR_SERVICE_ID 0x0003
#define MOTOR_INSTANCE_ID 0x0003
#define MOTOR_METHOD_ID 0x0003
#define SETMIN_METHOD_ID 0x0103
#define EMERGENCY_BREAK_EVENT_ID 0xE133
#define EMERGENCY_BREAK_EVENTGROUP_ID 0xE133

#define SPEED_SERVICE_ID 0x0033
#define SPEED_INSTANCE_ID 0x0033
#define SPEED_EVENT_ID 0xE033
#define SPEED_EVENTGROUP_ID 0xE033

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vsomeip/vsomeip.hpp>

class MotorSpeedService {
public:
    MotorSpeedService(uint32_t sp_sleep, uint8_t min_dist);
    bool init();
    void start();
    void stop();
    bool is_running();

private:
    bool run_;
    bool go_;
    bool use_dist_;
    uint32_t pub_sp_sleep_;
    uint8_t min_dist_;

    std::thread pub_sp_thread_;
    std::mutex mu_run_;
    std::condition_variable cond_run_;

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<vsomeip::payload> payload_;

    void run_sp();
    void on_dist_eve(const std::shared_ptr<vsomeip::message>& msg);
    void on_motor_req(const std::shared_ptr<vsomeip::message>& msg);
    void on_setmin_req(const std::shared_ptr<vsomeip::message>& msg);
    void on_shutdown(const std::shared_ptr<vsomeip::message>& msg);
    void on_go_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool go);
    void on_dist_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool use_dist);
    void on_state(vsomeip::state_type_e state);
};

#endif
