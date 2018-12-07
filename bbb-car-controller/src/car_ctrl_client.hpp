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


/**
 * @brief
 *
 *
 */
class CarCTRLClient {
public:

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_mo;

    /**
     * @brief
     *
     *
     */
    Buffer buf_mo;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_st;

    /**
     * @brief
     *
     *
     */
    Buffer buf_st;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_sp;

    /**
     * @brief
     *
     *
     */
    Buffer buf_sp;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_di;

    /**
     * @brief
     *
     *
     */
    Buffer buf_di;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_go;

    /**
     * @brief
     *
     *
     */
    Buffer buf_go;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_setmin;

    /**
     * @brief
     *
     *
     */
    Buffer buf_setmin;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_cam;

    /**
     * @brief
     *
     *
     */
    Buffer buf_cam;

    /**
     * @brief
     *
     *
     */
    CSharedMemory shm_shutdown;

    /**
     * @brief
     *
     *
     */
    Buffer buf_shutdown;

    /**
     * @brief
     *
     *
     */
    CarCTRLClient(uint32_t mo_sleep, uint32_t di_sleep, uint32_t setmin_sleep, bool skip_go);

    /**
     * @brief
     *
     *
     */
    bool init();

    /**
     * @brief
     *
     *
     */
    void start();

    /**
     * @brief
     *
     *
     */
    void stop();
private:

    /**
     * @brief
     *
     *
     */
    bool run_;

    /**
     * @brief
     *
     *
     */
    bool go_;

    /**
     * @brief
     *
     *
     */
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
