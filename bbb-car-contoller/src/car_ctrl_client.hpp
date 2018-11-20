#ifndef CAR_CTRL_CLIENT_HPP
#define CAR_CTRL_CLIENT_HPP

#define GO_SERVICE_ID 0x0001
#define GO_INSTANCE_ID 0x0001

#define STEER_SERVICE_ID 0x0002 
#define STEER_INSTANCE_ID 0x0002
#define STEER_METHOD_ID 0x0002

#define DIST_SERVICE_ID 0x0022 
#define DIST_INSTANCE_ID 0x0022
#define DIST_EVENT_ID 0xE022
#define DIST_EVENTGROUP_ID 0xE022

#define MOTOR_SERVICE_ID 0x0003
#define MOTOR_INSTANCE_ID 0x0003
#define MOTOR_METHOD_ID 0x0003

#define SPEED_SERVICE_ID 0x0033
#define SPEED_INSTANCE_ID 0x0033
#define SPEED_EVENT_ID 0xE033
#define SPEED_EVENTGROUP_ID 0xE033

#define DIST_Q_DEPTH 10
#define SPEED_Q_DEPTH 10

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vsomeip/vsomeip.hpp>

class CarCTRLClient {
public:
    CarCTRLClient(uint32_t n);
    bool init();
    void start();
    bool check_availability(vsomeip::service_t serv, vsomeip::instance_t inst);
    bool update_go_status();
    void send_motor_req(char s, bool prio);
    void send_motor_req(char s, char a, bool prio);
    void send_steer_req(char d, bool prio);
    char pop_speed();
    //char* pop_distance(); // TODO consider using vector type instead

private:
    bool is_init_;
    bool go_;
    bool app_busy_;
    bool is_ava_di_;
    bool is_ava_st_;
    bool is_ava_mo_;
    bool is_ava_sp_;
    uint32_t services_tot_;
    uint32_t services_disc_;

    std::queue<char> speed_q_;
    //std::queue<char*> dist_q_;

    std::mutex mu_app_;
    std::condition_variable cond_app_;
    std::mutex mu_sp_q_;
    std::condition_variable cond_sp_q_;
    std::mutex mu_di_q_;
    std::condition_variable cond_di_q_;

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<vsomeip::payload> payload_;
    std::shared_ptr<vsomeip::message> request_;

    void on_dist_eve(const std::shared_ptr<vsomeip::message>&);
    void on_speed_eve(const std::shared_ptr<vsomeip::message>&);
    void send_req(std::vector<vsomeip::byte_t> data,
                  vsomeip::service_t serv,
                  vsomeip::instance_t inst,
                  vsomeip::method_t meth);
    void on_state(vsomeip::state_type_e);
    void on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool);
};

#endif
