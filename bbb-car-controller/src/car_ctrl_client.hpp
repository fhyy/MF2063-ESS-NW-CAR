#ifndef CAR_CTRL_CLIENT_HPP
#define CAR_CTRL_CLIENT_HPP

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    #include <csignal>
#endif

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vsomeip/vsomeip.hpp>
#include "vsomeip_ids.hpp"
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

class CarCTRLClient {
public:

    CSharedMemory shm_mo;
    Buffer buf_mo;

    CSharedMemory shm_st;
    Buffer buf_st;

    CSharedMemory shm_sp;
    Buffer buf_sp;

    CSharedMemory shm_di;
    Buffer buf_di;

    CSharedMemory shm_go;
    Buffer buf_go;

    CSharedMemory shm_setmin;
    Buffer buf_setmin;

    CSharedMemory shm_cam;
    Buffer buf_cam;

    CSharedMemory shm_shutdown;
    Buffer buf_shutdown;

    CarCTRLClient(uint32_t mo_sleep, uint32_t di_sleep, uint32_t setmin_sleep, bool skip_go);
    bool init();
    void start();
    void stop();
private:
    bool run_;
    bool go_;
    bool skip_go_;
    bool app_busy_;
    bool is_ava_di_;
    bool is_ava_st_;
    bool is_ava_mo_;
    bool is_ava_sp_;
    bool is_ava_cam_;

    uint32_t req_mo_sleep_;
    uint32_t req_st_sleep_;
    uint32_t req_setmin_sleep_;
    uint32_t req_shutdown_sleep_;

    std::thread req_mo_thread_;
    std::thread req_st_thread_;
    std::thread req_setmin_thread_;
    std::thread req_shutdown_thread_;

    std::mutex mu_run_;
    std::condition_variable cond_run_;
    std::mutex mu_app_;
    std::condition_variable cond_app_;

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<vsomeip::payload> payload_;
    std::shared_ptr<vsomeip::message> request_;

    void update_go_status();
    void send_motor_req();
    void send_steer_req();
    void send_setmin_req();
    void send_shutdown_req();
    void on_dist_eve(const std::shared_ptr<vsomeip::message>&);
    void on_speed_eve(const std::shared_ptr<vsomeip::message>&);
    void on_cam_eve(const std::shared_ptr<vsomeip::message>&);
    void on_embreak_eve(const std::shared_ptr<vsomeip::message>&);
    void send_req(std::vector<vsomeip::byte_t> data,
                  vsomeip::service_t serv,
                  vsomeip::instance_t inst,
                  vsomeip::method_t meth);
    void on_state(vsomeip::state_type_e);
    void on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool);
};

#endif
