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
     *
     *
     */
    ESSPrototype();

    /**
     * @brief
     *
     *
     */
    void shutdown();

    /**
     * @brief
     * @param prio
     *
     *
     */
    void shutdown(bool prio);

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
    bool checkCameraSensor();

    /**
     * @brief
     * @param d
     * @param prio
     *
     */
    void setMinDistance(char d, bool prio);

    /**
     * @brief
     * @param d
     *
     *
     */
    void setMinDistance(char d);

    /**
     * @brief
     * @param s
     * @param prio
     *
     *
     */
    void setSpeed(char s, bool prio);

    /**
     * @brief
     * @param s
     *
     *
     */
    void setSpeed(char s);

    /**
     * @brief
     * @param s
     * @param a
     * @param prio
     *
     *
     */
    void setSpeed(char s, char a, bool prio);

    /**
     * @brief
     * @param s
     * @param a
     *
     *
     */
    void setSpeed(char s, char a);

    /**
     * @brief
     * @param d
     * @param prio
     *
     *
     */
    void setDirection(char d, bool prio);

    /**
     * @brief
     * @param d
     *
     *
     */
    void setDirection(char d);

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
