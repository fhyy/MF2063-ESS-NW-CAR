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
#define EMERGENCY_BREAK_EVENT_ID 0xE103
#define EMERGENCY_BREAK_EVENTGROUP_ID 0xE103

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

/**
 * @brief This class provides the speed sensor service and the motor actuator service.
 *
 * The speed service publishes (notifies) sensor data at a set interval. Another vsomeip
 * application may subscribe to this sevice to get regular speed sensor updates. The
 * motor service follows a client-server model where the client app sends a request
 * and the server (this class) sets the wheels to the requested angle. An instance of
 * this class also subscribes to distance events published by another application.
 * If the value of a distance event falls below a certain threshold, the motor service
 * publishes an "emergency break" event that other vsomeip applications may subscribe to.
 * @author Leon Fernandez
 */
class MotorSpeedService {
public:
    /**
     * @brief Constructor for the MotorSpeedService class.
     * @param sp_sleep Time (in millisec) that the speed notifier thread will sleep
     *        during each cycle.
     * @param min_dist The threshold value that determines when an emergency break
     *        event gets published
     * @author Leon Fernandez
     */
    MotorSpeedService(uint32_t sp_sleep, uint8_t min_dist);

    /**
     * @brief Initializer for a MotorSpeedService app/instance.
     * @return Returns true if the initialization was succesful, false otherwise
     *
     * Initializes the MotorSpeed app by registering @ref on_state, @ref on_steer_req,
     * @ref on_setmin_req, @ref on_go_availability, @ref on_dist_availability and @on_shutdown
     * as handlers for different vsomeip events. It also initializes @ref pub_sp_thread_.
     * @author Leon Fernandez
     */
    bool init();

    /**
     * @brief Starts the application by registering it to the vsomeip runtime environment.
     * @author Leon Fernandez
     */
    void start();

    /**
     * @brief Stops the application by joining all threads and deregistering from the vsomeip RTE
     * @author Leon Fernandez
     */
    void stop();

    /**
     * @brief Getter so external entities can read the run_ member.
     * @author Leon Fernandez
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
     * @brief Set to true if the distance service is up and can be used for emergency break events.
     *
     * If false, @ref min_dist_ is not used and emergency break events will never occur.
     */
    bool use_dist_;

    /**
     * @brief Time (in milliseconds) that the speed publisher thread will sleep between cycles.
     */
    uint32_t pub_sp_sleep_;

    /**
     * @brief The threshold for when an emergency break event is published
     */
    uint8_t min_dist_;

    /**
     * @brief The thread object for the function @ref run_sp.
     *
     * Thread that is instatiated inside @ref init. Continuously reads a sensor value then
     * publishes it for all subscribers to see. Sleeps for @ref pub_sp_sleep_ milliseconds
     * between cycles.
     */
    std::thread pub_sp_thread_;

    /**
     * @brief Mutex for synchronizing the start of all threads.
     */
    std::mutex mu_run_;

    /**
     * @brief Condition variable for synchronizing the start of all threads.
     */
    std::condition_variable cond_run_;

    /**
     * @brief VSOMEIP app that provides the speed and motor services.
     */
    std::shared_ptr<vsomeip::application> app_;

    /**
     * @brief Payload that is used for the VSOMEIP packets.
     */
    std::shared_ptr<vsomeip::payload> payload_;

    /**
     * @brief This method is the method run by @ref pub_sp_thread_.
     *
     * It basically waits for the initialization to complete then enters an
     * eternal loop. If the go-service is unavailable it spins around doing nothing.
     * If the go-service is available it publishes some data (preferably sensor data) and
     * then goes to sleep for a set amount of time.
     * @author Leon Fernandez
     */
    void run_sp();

    void on_dist_eve(const std::shared_ptr<vsomeip::message>& msg);
    void on_motor_req(const std::shared_ptr<vsomeip::message>& msg);
    void on_setmin_req(const std::shared_ptr<vsomeip::message>& msg);

    /**
     * @brief Function that is called when a message with SHUTDOWN_METHOD_ID arrives.
     * @param msg vsomeip message with SHUTDOWN_METHOD_ID
     *
     * Shuts down the application and the entire program by calling @ref stop.
     * @author Leon Fernandez
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
     * @author Leon Fernandez
     */
    void on_go_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool go);

    /**
     * @brief Function that is called when the availability of the distance-service changes.
     * @param serv ID of the service, should always be ==DIST_SERVICE_ID
     * @param inst ID of the specific service instance, should always be ==DIST_INSTANCE_ID
     * @param go True if the service is available, false otherwise
     *
     * Sets @ref use_dist_ to true if the dist service is available. This allows the motor
     * service to get sensor data directly from the distance service and thereby raise
     * emergency break events.
     * @author Leon Fernandez
     */

    void on_dist_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool use_dist);

    /**
     * @brief Function that is called when the registration state of the application changes.
     * @param state The incoming state that the application has within the vsomeip runtime.
     * 
     * Upon succesful registration within the vsomeip runtime it offers the speed and
     * motor services (makes them available) and requests association with the go-server
     * and with the distance server.
     * @author Leon Fernandez
     */
    void on_state(vsomeip::state_type_e state);
};

#endif































