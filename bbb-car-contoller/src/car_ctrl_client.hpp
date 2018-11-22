#ifndef CAR_CTRL_CLIENT_HPP
#define CAR_CTRL_CLIENT_HPP

#define DIST_Q_DEPTH 10
#define SPEED_Q_DEPTH 10

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    #include <csignal>
#endif

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vsomeip/vsomeip.hpp>

class CarCTRLClient {
public:
    CarCTRLClient();
    bool init();
    void start();
    void stop();
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
    void on_embreak_eve(const std::shared_ptr<vsomeip::message>&);
    void send_req(std::vector<vsomeip::byte_t> data,
                  vsomeip::service_t serv,
                  vsomeip::instance_t inst,
                  vsomeip::method_t meth);
    void on_state(vsomeip::state_type_e);
    void on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool);
};

#endif
