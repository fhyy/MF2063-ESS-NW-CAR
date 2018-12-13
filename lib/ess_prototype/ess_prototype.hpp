#ifndef ESS_PROTOTYPE_HPP
#define ESS_PROTOTYPE_HPP

#include <vector>
#include <iostream>
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

#define MO_MASK 0x00000001
#define ST_MASK 0x00000002
#define SP_MASK 0x00000004
#define DI_MASK 0x00000008
#define CAM_MASK 0x00000010

/**
 * @brief Simple class representing the current flag state
 *
 * The camera is tracking a flag and feeding a feature extraction
 * (color and position) into vsomeip. This class represent those
 * flag features.
 * @author Leon Fernandez
 */
class Flag {
public:

    /**
     * @brief Constructor for the flag class.
     *
     */
    Flag() : col(Null_Color), pos(Null_Position) {}

    /**
     * @brief Enumeration representing the flag's color states.
     *
     */
    enum Color {
        Red,
        Green,
        Yellow,
        Null_Color
    };

    /**
     * @brief Enumeration representing the flag's position states.
     *
     */
    enum Position {
        Left,
        Right,
        Middle,
        Null_Position
    };

    /**
     * @brief Color of the flag.
     *
     *
     */
    Color col;

    /**
     * @brief Position of the flag
     *
     *
     */
    Position pos;    
};


/**
 * @brief High-level library for writing car programs.
 *
 * This library aims to provide the developer with a set
 * of high-level functions which the developer can use
 * to build programs that control the car, either autonomously
 * or manually. A very simple example of such a program is the
 * ESS Shell (bbb-car-controller/src/ess_shell.cpp).
 * @author Leon Fernandez
 */
class ESSPrototype {
public:

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating speed sensor values
     */
    CSharedMemory shm_sp;
    /**
     * @brief Cyclic buffer for keeping track of speed values in the shared memory.
     */
    Buffer buf_sp;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating distance sensor values
     */
    CSharedMemory shm_di;
    /**
     * @brief Cyclic buffer for keeping track of distance values in the shared memory.
     */
    Buffer buf_di;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating which vsomeip services are up
     */
    CSharedMemory shm_go;
    /**
     * @brief Cyclic buffer for keeping track of service info values in the shared memory.
     */
    Buffer buf_go;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating camera sensor values
     */
    CSharedMemory shm_cam;
    /**
     * @brief Cyclic buffer for keeping track of camera values in the shared memory.
     */
    Buffer buf_cam;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating motor request values
     */
    CSharedMemory shm_mo;
    /**
     * @brief Cyclic buffer for keeping track of motor values in the shared memory.
     */
    Buffer buf_mo;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating steering request values
     */
    CSharedMemory shm_st;
    /**
     * @brief Cyclic buffer for keeping track of steering values in the shared memory.
     */
    Buffer buf_st;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating setmin request values
     */
    CSharedMemory shm_setmin;
    /**
     * @brief Cyclic buffer for keeping track of min-distance values in the shared memory.
     */
    Buffer buf_setmin;

    /**
     * @brief Shared memory used for communication between ess_prototype and vsomeip.
     *
     * This shared memory is for communicating shutdown request values
     */
    CSharedMemory shm_shutdown;
    /**
     * @brief Cyclic buffer for keeping track of shutdown requests in the shared memory.
     */
    Buffer buf_shutdown;

    /**
     * @brief Constructor for the ESSPrototype class.
     *
     * An object of the ESSPrototype class is meant to represent a self-aware car.
     * It can do basic car things such as: set the speed to a certain level, change
     * the angle of the wheels and check how fast it is going. But it is also aware
     * of its internal conditions in the sense that it can detect if any of the
     * basic services goes missing. For instance if the power to the camera node is
     * cut, an object of this class has the ability to detect this.
     */
    ESSPrototype();

    /**
     * @brief Shuts down all connected services.
     *
     * This method shuts down all connected vsomeip services with no way of bringing them
     * back online. Future developers could consider overloading this method so that
     * it can be used to shut down only a selected service.
     */
    void shutdown();

    /**
     * @brief Shuts down all connected services.
     * @param prio Sets the prio-flag in the packet
     *
     * This method shuts down all connected vsomeip services with no way of bringing them
     * back online. Future developers could consider overloading this method so that
     * it can be used to shut down only a selected service.
     */
    void shutdown(bool prio);

    /**
     * @brief Used to check the status of the motor service
     * @return True if the motor service was online the last time @ref getGoStatus() was called, false otherwise
     *
     */
    bool checkMotor();

    /**
     * @brief Used to check the status of the speed sensor service
     * @return True if the speed sensor service was online the last time @ref getGoStatus() was called,
               false otherwise
     *
     */
    bool checkSpeedSensor();

    /**
     * @brief Used to check the status of the steering service
     * @return True if the steering service was online the last time @ref getGoStatus() was called, false otherwise
     *
     */
    bool checkSteering();

    /**
     * @brief Used to check the status of the distance sensor service
     * @return True if the distance sensor service was online the last time @ref getGoStatus()
     *         was called, false otherwise
     *
     */
    bool checkDistanceSensor();

    /**
     * @brief Used to check the status of the camera service
     * @return True if the camera service was online the last time @ref getGoStatus() was called, false otherwise
     *
     */
    bool checkCameraSensor();

    /**
     * @brief Sets the threshold for when EMERGENCY BREAK (EMBREAK) events are raised
     * @param d Distance (in cm) of the minimum allowable threshold
     * @param prio Sets the prio flag in the packet
     *
     */
    void setMinDistance(char d, bool prio);

    /**
     * @brief Sets the threshold for when EMERGENCY BREAK (EMBREAK) events are raised
     * @param d Distance (in cm) of the minimum allowable threshold
     *
     * Overloaded version which always sets prio to false.
     */
    void setMinDistance(char d);

    /**
     * @brief Sends the desired speed to the motor controller
     * @param s The desired speed (in cm/s)
     * @param prio sets the prio flag in the packet
     *
     *
     */
    void setSpeed(char s, bool prio);

    /**
     * @brief Sends the desired speed to the motor controller
     * @param s The desired speed (in cm/s)
     *
     * Overloaded version which always sets prio to false
     */
    void setSpeed(char s);

    /**
     * @brief Sends the desired speed to the motor controller
     * @param s The desired speed (in cm/s)
     * @param a The desired acceleration
     * @param prio sets the prio flag in the packet
     *
     * Sets the speed to "s" with a certain acceleration "a". NOT IMPLEMENTED ON
     * MOTOR CONTROLLER!
     */
    void setSpeed(char s, char a, bool prio);

    /**
     * @brief Sends the desired speed to the motor controller
     * @param s The desired speed (in cm/s)
     * @param a The desired acceleration
     *
     * Same as the above but prio is always false.
     */
    void setSpeed(char s, char a);

    /**
     * @brief Sets the wheels to the desired direction
     * @param d The direction of the wheels
     * @param prio sets the prio flag in the packet
     *
     * 0 means full left, 100 means full right 50 is straight ahead.
     */
    void setDirection(char d, bool prio);

    /**
     * @brief Sets the wheels to the desired direction
     * @param d The direction of the wheels
     *
     * Same as the other version, but prio is always false.
     */
    void setDirection(char d);

    /**
     * @brief Get the curren speed.
     * @return Returns the current speed.
     *
     *
     */
    unsigned char getSpeed();

    /**
     * @brief Get the free distance ahead.
     * @return The free distance ahead.
     *
     * Every speed sensor packet consists of three measurements, one from each sensor.
     * This function returns the SMALLEST of those three values. Future developers could
     * consider overloading this function in a way that allows any of the three values to be read.
     */
    unsigned char getDistance();

    /**
     * @brief Method for checking if the car is ready to go or not.
     * @return True if all vsomeip services are up, false otherwise
     *
     * Loads the latest status of all vsomeip services in the form of a
     * one-hot encoded int. It is important to know that @ref checkMotor, @ref checkSteering
     * @ref checkSpeedSensor, @ref checkCameraSensor and @ref checkDistanceSensor WILL NEVER UPDATE
     * THEIR RETURN VALUE UNLESS THIS FUNCTION IS CALLED BETWEEN INVOCATIONS.
     */
    bool getGoStatus();


    /**
     * @brief Gets the current status of the flag
     * @return an object of type @ref Flag, representing the features which the camera extracts.
     *
     *
     */
    Flag getFlag();

private:

    /**
     * @brief A one-hot encoded int where every bit represents the availability of a service
     *
     * This member is updated every time @see CarCTRLClient#update_go_status is called and it holds
     * the information of what services where available at the time. @ref checkMotor, @ref checkSteering
     * @ref checkSpeedSensor, @ref checkCameraSensor and @ref checkDistanceSensor only read from
     * this int when they are called, which means that @see CarCTRLClient#update_go_status has to be called in order
     * to update the output of the check-methods.
     */
    int service_status_;

    /**
     * @brief The latest distance read from the sensors.
     *
     */
    int dist_latest_;

    /**
     * @brief The latest speed read from the sensors.
     *
     */
    unsigned char speed_latest_;

    /**
     * @brief The latest flag read from the sensors.
     *
     */
    Flag flag_latest_;
};

#endif
