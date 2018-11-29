#include "ess_prototype.hpp"

#define DEBUG 1

ESSPrototype::ESSPrototype() :
    service_status_(0),
    dist_latest_(0),
    speed_latest_(0), // TODO flag_latest_
    shm_sp(CSharedMemory("/shm_sp")),
    shm_di(CSharedMemory("/shm_di")),
    shm_go(CSharedMemory("/shm_go")),
    shm_cam(CSharedMemory("/shm_cam")),
    shm_mo(CSharedMemory("/shm_mo")),
    shm_st(CSharedMemory("/shm_st")),
    shm_setmin(CSharedMemory("/shm_setmin")),
    shm_shutdown(CSharedMemory("/shm_shutdown"))
{

    #if (DEBUG)
        std::cout << "## DEBUG ## ess_prototype initializing consumer memory ## DEBUG ##"
                  << std::endl;
    #endif

    int *p;

    shm_sp.Create(BUFFER_SIZE, O_RDWR);
    shm_sp.Attach(PROT_WRITE);
    p = (int*) shm_sp.GetData();
    buf_sp = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shm_di.Create(BUFFER_SIZE, O_RDWR);
    shm_di.Attach(PROT_WRITE);
    p = (int*) shm_di.GetData();
    buf_di = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shm_go.Create(BUFFER_SIZE, O_RDWR);
    shm_go.Attach(PROT_WRITE);
    p = (int*) shm_go.GetData();
    buf_go = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shm_cam.Create(BUFFER_SIZE, O_RDWR);
    shm_cam.Attach(PROT_WRITE);
    p = (int*) shm_cam.GetData();
    buf_cam = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    sleep(5);

    #if (DEBUG)
        std::cout << "## DEBUG ## ess_prototype initializing producer memory ## DEBUG ##"
                  << std::endl;
    #endif

    shm_mo.Create(BUFFER_SIZE, O_RDWR);
    shm_mo.Attach(PROT_WRITE);
    p = (int*) shm_mo.GetData();
    buf_mo = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shm_st.Create(BUFFER_SIZE, O_RDWR);
    shm_st.Attach(PROT_WRITE);
    p = (int*) shm_st.GetData();
    buf_st = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shm_setmin.Create(BUFFER_SIZE, O_RDWR);
    shm_setmin.Attach(PROT_WRITE);
    p = (int*) shm_setmin.GetData();
    buf_setmin = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shm_shutdown.Create(BUFFER_SIZE, O_RDWR);
    shm_shutdown.Attach(PROT_WRITE);
    p = (int*) shm_shutdown.GetData();
    buf_shutdown = Buffer(BUFFER_SIZE, p, B_PRODUCER);
}

void ESSPrototype::shutdown(bool prio) {
    int data = 1 | ((prio && 0xFF) << 24);
    shm_shutdown.Lock();
    buf_shutdown.write(data);
    shm_shutdown.UnLock();
}

void ESSPrototype::shutdown() {
    shutdown(false);
}

bool ESSPrototype::checkMotor() {
    return service_status_ & MO_MASK;
}

bool ESSPrototype::checkSpeedSensor() {
    return service_status_ & SP_MASK;
}

bool ESSPrototype::checkSteering() {
    return service_status_ & ST_MASK;
}

bool ESSPrototype::checkDistanceSensor() {
    return service_status_ & DI_MASK;
}

bool ESSPrototype::checkCameraSensor() {
    return service_status_ & CAM_MASK;
}

void ESSPrototype::setSpeed(char s, char a, bool prio) {
    int data = s | (a<<8) | ((prio && 0xFF) << 24);
    shm_mo.Lock();
    buf_mo.write(data);
    shm_mo.UnLock();
}

void ESSPrototype::setSpeed(char s, bool prio) {
    ESSPrototype::setSpeed(s, 0, prio);
}

void ESSPrototype::setSpeed(char s, char a) {
    ESSPrototype::setSpeed(s, a, false);
}

void ESSPrototype::setSpeed(char s) {
    ESSPrototype::setSpeed(s, 0, false);
}

void ESSPrototype::setDirection(char d, bool prio) {
    int data = d | ((prio && 0xFF) << 24);
    shm_st.Lock();
    buf_st.write(data);
    shm_st.UnLock();
}

void ESSPrototype::setDirection(char d) {
    setDirection(d, false);
}

void ESSPrototype::setMinDistance(char d, bool prio) {
    int data = d | (prio && 0xFF << 24);
    shm_setmin.Lock();
    buf_setmin.write(data);
    shm_setmin.UnLock();
}

void ESSPrototype::setMinDistance(char d) {
    setMinDistance(d, false);
}

char ESSPrototype::getSpeed() {
    std::vector<int> data;

    shm_sp.Lock();
    int unreadValues = buf_sp.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_sp.read());
    shm_sp.UnLock();

    if (data.size() > 0)
        speed_latest_ = (unsigned char) data.back();

    return speed_latest_;
}

char ESSPrototype::getDistance() {
    std::vector<int> data;
    shm_di.Lock();
    int unreadValues = buf_di.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_di.read());
    shm_di.UnLock();

    if (data.size() > 0) {
        int latest_i = data.back();
        char *latest = (char*) &latest_i; //TODO does this work?
        if (latest[0] < latest[1] && latest[0] < latest[2])
            dist_latest_ = (char) latest[0];
        else if (latest[1] < latest[0] && latest[1] < latest[2])
            dist_latest_ = latest[1];
        else // latest[2] is smallest
            dist_latest_ = latest[2];
    }

    return dist_latest_;
}

/*Flag ESSPrototype::getFlag() {
    std::vector<int> data;
    shm_di.Lock();
    int unreadValues = buf_di.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_di.read());
    shm_di.UnLock();

    int latest = data.back();

    TODO decode latest and create flag object

    return flag;
}*/

bool ESSPrototype::getGoStatus() {
    std::vector<int> data;

    shm_go.Lock();
    int unreadValues = buf_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_go.read());
    shm_go.UnLock();

    if (data.size() > 0)
        service_status_ = data.back();

    return service_status_ == 0x0000001F;
}
