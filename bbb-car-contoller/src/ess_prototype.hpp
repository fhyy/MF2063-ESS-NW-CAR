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

class ESSPrototype {
public:
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

    CSharedMemory shmMemory_mo;
    Buffer circBuffer_mo;

    CSharedMemory shmMemory_st;
    Buffer circBuffer_st;

    CSharedMemory shmMemory_sp;
    Buffer circBuffer_sp;

    CSharedMemory shmMemory_di;
    Buffer circBuffer_di;

    CSharedMemory shmMemory_setmin;
    Buffer circBuffer_setmin;

    CSharedMemory shmMemory_go;
    Buffer circBuffer_go;

    CSharedMemory shmMemory_cam;
    Buffer circBuffer_cam;

    CSharedMemory shmMemory_shutdown;
    Buffer circBuffer_shutdown;
};

#endif
