#ifndef DIST_STEER_SERVICE_HPP
#define DIST_STEER_SERVICE_HPP

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    #include <csignal>
#endif

#include <iostream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vsomeip/vsomeip.hpp>
#include "vsomeip_ids.hpp"
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

/**
 * @brief This class provides the distance sensor service and the steering actuator service.
 *
 * The distance service publishes (notifies) sensor data at a set interval. Another vsomeip
 * application may subscribe to this sevice to get regular distance sensor updates. The
 * steering service follows a client-server model where the client app sends a request
 * and the server (this class) sets the wheels to the requested angle.
 * @author Leon Fernandez
 */
class DistSteerService {
public:

    /**
     * @brief Shared memory used for IPC between the distance sensor reader program and this class.
     */
    CSharedMemory shm_di;

    /**
     * @brief Cyclic buffer for keeping track of distance values in the shared memory.
     */
    Buffer buf_di;

    /**
     * @brief Shared memory used for IPC between this class and the steering control program.
     */
    CSharedMemory shm_st;

    /**
     * @brief Cyclic buffer for keeping track of requested steering values in the shared memory.
     */
    Buffer buf_st;

    /**
     * @brief Constructor for the DistSteerService class.
     * @param di_sleep Time (in millisec) that the distance notifier thread will sleep
     *        during each cycle.
     * @param skip_go Allows threads to ignore the value @ref go_. Only for testing purposes!
     */
    DistSteerService(uint32_t di_sleep, bool skip_go);

    /**
     * @brief Initializer for a DistSteerService app/instance.
     * @return Returns true if the initialization was succesful, false otherwise
     *
     * Initializes the DistSteering app by registering @ref on_state, @ref on_steer_req and
     * @ref on_go_availability as handlers for different vsomeip events. It also initializes
     * @ref pub_di_thread_.
     */
    bool init();

    /**
     * @brief Starts the application by registering it to the vsomeip runtime environment.
     * @author Leon Fernandez
     */
    void start();

    /**
     * @brief Stops the application by joining all threads and deregistering from the vsomeip RTE
     */
    void stop();

    /**
     * @brief Getter so external entities can read the run_ member.
     */
    bool is_running();
private:

    /**
     * @brief Main on/off switch for application.
     *
     * Set to true when initialization is succesfully completed. Set to false when @ref stop is
     * called, causing all threads to run to completion so they can be joined.
     */
    bool run_;

    /**
     * @brief Pause switch for all threads.
     *
     * All threads will just loop in one place without doing anything if set to false.
     * Set to true when a remote server (the go-server) comes online. Set to false
     * if the go-server goes offline or if @ref stop is called. Initial value is set to false.
     */
    bool go_;

    /**
     * @brief Disables pause switch (@ref go_) for all threads.
     *
     * Can only be set by passing an argument at the cmd line when starting the program.
     */
    bool skip_go_;

    /**
     * @brief Time (in milliseconds) that the distance publisher thread will sleep between cycles.
     */
    uint32_t pub_di_sleep_;

    /**
     * @brief The thread object for the function @ref run_di.
     *
     * Thread that is instatiated inside @ref init. Continuously reads a sensor value then
     * publishes it for all subscribers to see. Sleeps for @ref pub_di_sleep_ milliseconds
     * between cycles.
     */
    std::thread pub_di_thread_;

    /**
     * @brief Mutex for synchronizing the start of all threads.
     */
    std::mutex mu_run_;

    /**
     * @brief Condition variable for synchronizing the start of all threads.
     */
    std::condition_variable cond_run_;

    /**
     * @brief VSOMEIP app that provides the dist and steer services.
     */
    std::shared_ptr<vsomeip::application> app_;

    /**
     * @brief Payload that is used for the VSOMEIP packets.
     */
    std::shared_ptr<vsomeip::payload> payload_;

    /**
     * @brief This method is the method run by @ref pub_di_thread_.
     *
     * It basically waits for the initialization to complete then enters an
     * eternal loop. If the go-service is unavailable it spins around doing nothing.
     * If the go-service is available it publishes some data (preferably sensor data) and
     * then goes to sleep for a set amount of time.
     */
    void run_di();

    /**
     * @brief Function that is called when a message with STEER_METHOD_ID arrives.
     * @param msg vsomeip message with STEER_METHOD_ID and a payload representing the
     *            desired angle for the wheels.
     *
     * Unpacks and writes the received request data to a shared memory, which can then be
     * read by the process controlling the steering.
     */
    void on_steer_req(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Function that is called when a message with SHUTDOWN_METHOD_ID arrives.
     * @param msg vsomeip message with SHUTDOWN_METHOD_ID
     *
     * Shuts down the application and the entire program by calling @ref stop.
     */
    void on_shutdown(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Function that is called when the availability of the go-service changes.
     * @param serv ID of the service, should always be ==GO_SERVICE_ID
     * @param inst ID of the specific service instance, should always be ==GO_INSTANCE_ID
     * @param go True if the service is available, false otherwise
     *
     * The go-service is a service made available by the central node (bbb-car-controller)
     * when that node has a preset number of services (ie when it has discovered all actuators).
     * The availability of the go-service basically works as a pause/play switch for this Class.
     */
    void on_go_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool go);

    /**
     * @brief Function that is called when the registration state of the application changes.
     * @param state The incoming state that the application has within the vsomeip runtime.
     * 
     * Upon succesful registration within the vsomeip runtime it offers the steering and
     * distance services (makes them available) and requests association with the go-server.
     */
    void on_state(vsomeip::state_type_e state);
};

#endif
