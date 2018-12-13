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
     * @brief Closes down the application. Unregisters from the vsomeip RTE.
     *
     *
     */
    void stop();
private:

    /**
     * @brief Main on/off switch for application.
     *
     * Set to true when initialization is succesfully completed. Set to false when @ref stop is
     * called, causing all threads to run to completion so they can be joined.
     */
    run_;

    /**
     * @brief Pause switch for all threads.
     *
     * All threads will just loop in one place without doing anything if set to false.
     * Set to true when a remote server (the go-server) comes online. Set to false
     * if the go-server goes offline or if @ref stop is called. Initial value is set to false.
     * This class is in charge if bringing the go-server up/down.
     */
    bool go_;

    /**
     * @brief Disables pause switch (@ref go_) for all threads.
     *
     * Can only be set by passing an argument at the cmd line when starting the program.
     * Note that the go-server will still be brought up/down like normal depending on available
     * services on the network.
     */
    bool skip_go_;

    /**
     * @brief Boolean that signals to all threads whether the app is in use.
     *
     * @ref app_ is a shared resource among all threads, therefore it is protected
     * by @ref mu_app_ and @ref cond_app_. This member is used in conjunction with
     * @ref cond_app_ to prevent sporadic wakeups while waiting on the mutex.
     */
    bool app_busy_;

     /**
     * @brief Indicates whether the distance sensor service is available or not.
     *
     *
     */
    bool is_ava_di_;

     /**
     * @brief Indicates whether the steering service is available or not.
     *
     *
     */
    bool is_ava_st_;

     /**
     * @brief Indicates whether the motor control service is available or not.
     *
     *
     */
    bool is_ava_mo_;

     /**
     * @brief Indicates whether the speedometer service is available or not.
     *
     *
     */
    bool is_ava_sp_;

     /**
     * @brief Indicates whether the camera service is available or not.
     *
     *
     */
    bool is_ava_cam_;

    /**
     * @brief Time (in millisec) that the motor request sender thread will sleep between cycles.
     */
    uint32_t req_mo_sleep_;

    /**
     * @brief Time (in millisec) that the steering request sender thread will sleep between cycles.
     */
    uint32_t req_st_sleep_;

    /**
     * @brief Time (in millisec) that the setmin request sender thread will sleep between cycles.
     */
    uint32_t req_setmin_sleep_;

    /**
     * @brief Time (in millisec) that the shutdown request sender thread will sleep between cycles.
     */
    uint32_t req_shutdown_sleep_;

    /**
     * @brief The thread object for the function @ref send_motor_req.
     *
     * Thread that is instatiated inside @ref init. Continuously reads a shared
     * memory which holds motor requests and then sends those requests.
     */
    std::thread req_mo_thread_;

    /**
     * @brief The thread object for the function @ref send_steer_req.
     *
     * Thread that is instatiated inside @ref init. Continuously reads a shared
     * memory which holds steering requests and then sends those requests.
     */
    std::thread req_st_thread_;

    /**
     * @brief The thread object for the function @ref send_setmin_req.
     *
     * Thread that is instatiated inside @ref init. Continuously reads a shared
     * memory which holds setmin requests and then sends those requests.
     */
    std::thread req_setmin_thread_;

    /**
     * @brief The thread object for the function @ref send_shutdown_req.
     *
     * Thread that is instatiated inside @ref init. Continuously reads a shared
     * memory which holds shutdown requests and then sends those requests.
     */
    std::thread req_shutdown_thread_;

    /**
     * @brief Mutex for synchronizing the start of all threads.
     */
    std::mutex mu_run_;

    /**
     * @brief Condition variable for synchronizing the start of all threads.
     */
    std::condition_variable cond_run_;

    /**
     * @brief Mutex to protect the vsomeip app, which is a shared resource.
     *
     * @ref app_ is a shared resource among all threads, therefore it is protected
     * by a this member and @ref cond_app_.
     */
    std::mutex mu_app_;

    /**
     * @brief Condition variable for waiting on the vsomeip app, which is a shared resource.
     *
     * @ref app_ is a shared resource among all threads, therefore it is protected
     * by a this member and @ref mu_app_.
     */
    std::condition_variable cond_app_;

    /**
     * @brief The vsomeip application which is used to send and receive messages.
     *
     * This member is a shared resource among the threads and therefore it is
     * protected by @ref mu_app_, @ref cond_app_ and @ref app_busy_.
     */
    std::shared_ptr<vsomeip::application> app_;

    /**
     * @brief Payload that is used for the VSOMEIP requests.
     */
    std::shared_ptr<vsomeip::payload> payload_;

    /**
     * @brief The message that contains @ref payload_.
     */
    std::shared_ptr<vsomeip::message> request_;


    /**
     * @brief Handler for bringing up/down the go-server.
     *
     * This member is called by @ref on_availability everytime the availability 
     * of services on the network changes. Depending on what is available it either
     * brings up or down the go-server and sets @ref go_ to the corresponding
     * value. It also communicates with the @ref ESSPrototype class over a 
     * shared memory so @ref ESSPrototype can see the availability of services
     * as well.
     */
    void update_go_status();

    /**
     * @brief This method is the method run by @ref req_mo_thread_.
     *
     * It basically waits for the initialization to complete then enters an
     * eternal loop. If the go-service is unavailable it spins around doing nothing.
     * If the go-service is available it sends a request that it reads from a shared
     * memory then goes to sleep for @ref req_mo_sleep_
     */
    void send_motor_req();

    /**
     * @brief This method is the method run by @ref req_st_thread_.
     *
     * It basically waits for the initialization to complete then enters an
     * eternal loop. If the go-service is unavailable it spins around doing nothing.
     * If the go-service is available it sends a request that it reads from a shared
     * memory then goes to sleep for @ref req_st_sleep_
     */
    void send_steer_req();

    /**
     * @brief This method is the method run by @ref req_setmin_thread_.
     *
     * It basically waits for the initialization to complete then enters an
     * eternal loop. If the go-service is unavailable it spins around doing nothing.
     * If the go-service is available it sends a request that it reads from a shared
     * memory then goes to sleep for @ref req_setmin_sleep_
     */
    void send_setmin_req();

    /**
     * @brief This method is the method run by @ref req_mo_thread_.
     *
     * It basically waits for the initialization to complete then enters an
     * eternal loop. If the go-service is unavailable it spins around doing nothing.
     * If the go-service is available it sends a request that it reads from a shared
     * memory then goes to sleep for @ref req_shutdown_sleep_
     */
    void send_shutdown_req();

    /**
     * @brief Method that is called when a distance event is detected.
     * @param msg vsomeip notification with distance sensor data.
     *
     * Receives distance sensor data then writes it to the corresponding
     * shared memory.
     */
    void on_dist_eve(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Method that is called when a speedometer event is detected.
     * @param msg vsomeip notification with speedometer data.
     *
     * Receives speedometer data then writes it to the corresponding
     * shared memory.
     */
    void on_speed_eve(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Method that is called when a camera event is detected.
     * @param msg vsomeip notification with camera sensor data.
     *
     * Receives camera sensor data then writes it to the corresponding
     * shared memory.
     */
    void on_cam_eve(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Method that is called when an embreak event is detected.
     * @param msg vsomeip notification with dummy data.
     *
     * Currently does nothing except prints a message to std::cout.
     */
    void on_embreak_eve(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Sends a request to some vsomeip.
     * @param data Payload of the request.
     * @param serv Which service that will handle the request.
     * @param inst Which instance of the service that will handle the request.
     * @param meth Which method that will be invoked when this message is received.
     *
     * This method should always be in a block protected by @ref mu_app_!
     */
    void send_req(std::vector<vsomeip::byte_t> data,
                  vsomeip::service_t serv,
                  vsomeip::instance_t inst,
                  vsomeip::method_t meth);

    /**
     * @brief Method that is called when the registration state of the application changes.
     * @param state The incoming state that the application has within the vsomeip runtime.
     * 
     * Upon succesful registration within the vsomeip runtime it requests association with
     * the steering, motor, speedometer, distance sensor and camera services as well as
     * offer the go-service if all these services are available.
     */
    void on_state(vsomeip::state_type_e);

    /**
     * @brief Method that is called when the availability of the distance-service changes.
     * @param serv ID of the service, should always be ==DIST_SERVICE_ID
     * @param inst ID of the specific service instance, should always be ==DIST_INSTANCE_ID
     * @param is_ava True if the service is available, false otherwise
     *
     * Sets @ref is_ava_st_, @ref is_ava_mo_, @ref is_ava_sp_, @ref is_ava_di_ and
     * @ref is_ava_cam_ to true/false depending on what is available on the network
     * then it calls @ref update_go_status.
     */
    void on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool is_ava);
};

#endif
