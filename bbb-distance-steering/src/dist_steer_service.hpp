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

/**
 * @brief This class provides the distance sensor service and the steering actuator service.
 *
 * The distance service publishes (notifies) sensor data at a set interval. Another vsomeip
 * application may subscribe to this sevice to get regular distance sensor updates. The
 * steering service follows a client-server model where the client app sends a request
 * and the server (this class) sets the wheels to the requested angle.
 */
class DistSteerService {
public:
    /**
     * @brief Constructor for the DistSteerService class.
     * @param di_sleep Time (in millisec) that the distance notifier thread will sleep
     *        during each cycle.
     */
    DistSteerService(uint32_t di_sleep);

    /**
     * @brief Initializer for a DistSteerService app/instance.
     * @return Returns true if the initialization was succesful, false otherwise
     *
     * Initializes the DistSteering app by registering @ref on_state, @ref on_steer_req and
     * @ref on_go as handlers for different vsomeip events. It also initializes
     * @ref pub_di_thread_.
     * 
     */
    bool init();

    /**
     * @brief Starts the application by registering it to the vsomeip runtime environment.
     */
    void start();

    /**
     * @brief Function that is called when the registration state of the application changes.
     * @param state The incoming state that the application has within the vsomeip runtime.
     * 
     * Upon succesful registration within the vsomeip runtime it offers the steering and
     * distance services (makes them available) and requests association with the go-server.
     */
    void on_state(vsomeip::state_type_e state);

    /**
     * @brief Function that is called when a message with STEER_METHOD_ID arrives.
     * @param msg vsomeip message with STEER_METHOD_ID and a payload representing the
     *            desired angle for the wheels.
     *
     * Currently does nothing except print a message.
     */
    void on_steer_req(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Function that is called when the availability of the go-service changes.
     * @param serv ID of the service, should always be ==GO_SERVICE_ID
     * @param inst ID of the specific service instance, should always be ==GO_INSTANCE_ID
     * @param go True if the service
     *
     * The go-service is a service made available by the central node (bbb-car-controller)
     * when that node has a preset number of services (ie when it has discovered all actuators).
     * The availability of the go-service basically works as a pause/play switch for this Class.
     */
    void on_go(vsomeip::service_t serv, vsomeip::instance_t inst, bool go);

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
     * @brief The thread object for the function @ref run_di.
     *
     * Thread that is instatiated inside @ref init.
     */
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
