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
 * @brief This class is the client that controls the car
 *
 * This class handles the VSOMEIP communication between nodes. It
 * subscribes to a number of event publishers and requests association with
 * the steering service and the motor service. For communicating with the
 * higher-level library @ref ESSPrototype it uses a number of shared memory
 * locations to write and read sensor values and requests, respectively.
 * It is worth noting that ESSPrototype can be replaced with some other software
 * as long as that software still writes to the same shared memories.
 * @author Leon Fernandez
 */
class CarCTRLClient {
public:

    /**
     * @brief Shared memory for reading motor requests, which are then sent.
     *
     *
     */
    CSharedMemory shm_mo;

    /**
     * @brief Buffer for keeping track of motor requests.
     *
     *
     */
    Buffer buf_mo;

    /**
     * @brief Shared memory for reading steering requests, which are then sent.
     *
     *
     */
    CSharedMemory shm_st;

    /**
     * @brief Buffer for keeping track of steering requests.
     *
     *
     */
    Buffer buf_st;

    /**
     * @brief Shared memory to which speed sensor data is written.
     *
     *
     */
    CSharedMemory shm_sp;

    /**
     * @brief Buffer for keeping track of speed data values.
     *
     *
     */
    Buffer buf_sp;

    /**
     * @brief Shared memory to which distance sensor data is written.
     *
     *
     */
    CSharedMemory shm_di;

    /**
     * @brief Buffer for keeping track of distance data values.
     *
     *
     */
    Buffer buf_di;

    /**
     * @brief Shared memory to which the service status is written.
     *
     *
     */
    CSharedMemory shm_go;

    /**
     * @brief Buffer for keeping track of service status data.
     *
     *
     */
    Buffer buf_go;

    /**
     * @brief Shared memory for reading setmin requests, which are then sent.
     *
     *
     */
    CSharedMemory shm_setmin;

    /**
     * @brief Buffer for keeping track of setmin values
     *
     *
     */
    Buffer buf_setmin;

    /**
     * @brief Shared memory to which camera sensor data is written.
     *
     *
     */
    CSharedMemory shm_cam;

    /**
     * @brief  Buffer for keeping track of camera sensor data.
     *
     *
     */
    Buffer buf_cam;

    /**
     * @brief Shared memory for reading shutdown requests, which are then sent.
     *
     *
     */
    CSharedMemory shm_shutdown;

    /**
     * @brief Buffer for keeping track of shutdown requests.
     *
     *
     */
    Buffer buf_shutdown;

    /**
     * @brief Constructor for the class. Also initializes the shared memory.
     *
     *
     */
    CarCTRLClient(uint32_t mo_sleep, uint32_t di_sleep, uint32_t setmin_sleep, bool skip_go);

    /**
     * @brief Initializes the vsomeip application.
     *
     *
     */
    bool init();

    /**
     * @brief Registers the vsomeip application to the vsomeip RTE.
     *
     *
     */
    void start();

    /**
     * @brief Closes down the application.
     *
     *
     */
    void stop();
private:

    /**
     * @brief Main on/off switch for the threads.
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
