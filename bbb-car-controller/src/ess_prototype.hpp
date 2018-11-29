#ifndef ESS_PROTOTYPE_HPP
#define ESS_PROTOTYPE_HPP

#include <vector>
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

class Flag {
public:
    std::string color;
    std::string position;
};

// TODO overload methods so that prio does not have to be passed
class ESSPrototype {
public:

    CSharedMemory shm_sp;
    Buffer buf_sp;

    CSharedMemory shm_di;
    Buffer buf_di;

    CSharedMemory shm_go;
    Buffer buf_go;

    CSharedMemory shm_cam;
    Buffer buf_cam;

    CSharedMemory shm_mo;
    Buffer buf_mo;

    CSharedMemory shm_st;
    Buffer buf_st;

    CSharedMemory shm_setmin;
    Buffer buf_setmin;

    CSharedMemory shm_shutdown;
    Buffer buf_shutdown;

    /**
     * @brief
     * @param ccc
     *
     *
     */
    ESSPrototype();

    //TODO bool boot();

    void shutdown();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkMotor();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkSpeedSensor();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkSteering();

    /**
     * @brief
     * @return
     *
     *
     */
    bool checkDistanceSensor();

    /**
     * @brief
     * @return
     *
     *
     */
    void setMinDistance(char d, bool prio);

    /**
     * @brief
     * @param s
     * @param prio
     * @return
     *
     *
     */
    void setSpeed(char s, bool prio);

    /**
     * @brief
     * @param s
     * @param a
     * @param prio
     * @return
     *
     *
     */
    void setSpeed(char s, char a, bool prio);

    /**
     * @brief
     * @param d
     * @param prio
     * @return
     *
     *
     */
    void setDirection(char d, bool prio);

    /**
     * @brief
     * @return
     *
     *
     */
    char getSpeed();

    /**
     * @brief
     * @return
     *
     *
     */
    char getDistance();

    /**
     * @brief
     * @return
     *
     *
     */
    bool getGoStatus();


    /**
     * @brief
     * @return
     *
     *
     */
    Flag getFlag();

private:

    int service_status_;

};

#endif
