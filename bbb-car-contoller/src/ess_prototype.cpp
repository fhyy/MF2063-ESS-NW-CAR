#include "ess_prototype.hpp"

ESSPrototype::ESSPrototype() :
    shm_sp(CSharedMemory("/shm_sp")),
    shm_di(CSharedMemory("/shm_di")),
    shm_go(CSharedMemory("/shm_go")),
    shm_cam(CSharedMemory("/shm_cam")),
    shm_mo(CSharedMemory("/shm_mo")),
    shm_st(CSharedMemory("/shm_st")),
    shm_setmin(CSharedMemory("/shm_setmin")),
    shm_shutdown(CSharedMemory("/shm_shutdown"))
{
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

    sleep(3);

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

//TODO bool EssPrototype::boot() {}

void ESSPrototype::shutdown() {
    int data = 1;
    shm_shutdown.Lock();
    buf_shutdown.write(data);
    shm_shutdown.UnLock();
}

bool ESSPrototype::checkMotor() {
    int mo_mask = 0x00000001;
    std::vector<int> data;

    shm_go.Lock();
    int unreadValues = buf_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_go.read());
    shm_go.UnLock();

    service_status_ = data.back();

    return service_status_ && mo_mask;
    //TODO make sure last element is the latest
}

bool ESSPrototype::checkSpeedSensor() {
    int sp_mask = 0x00000100;
    std::vector<int> data;

    shm_go.Lock();
    int unreadValues = buf_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_go.read());
    shm_go.UnLock();

    service_status_ = data.back();

    return service_status_ && sp_mask;
    //TODO make sure last element is the latest
}

bool ESSPrototype::checkSteering() {
    int st_mask = 0x00000010;
    std::vector<int> data;

    shm_go.Lock();
    int unreadValues = buf_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_go.read());
    shm_go.UnLock();

    service_status_ = data.back();

    return service_status_ && st_mask;
    //TODO make sure last element is the latest
}

bool ESSPrototype::checkDistanceSensor() {
    int di_mask = 0x00001000;
    std::vector<int> data;

    shm_go.Lock();
    int unreadValues = buf_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_go.read());
    shm_go.UnLock();

    service_status_ = data.back();

    return service_status_ && di_mask;
    //TODO make sure last element is the latest
}

void ESSPrototype::setSpeed(char s, bool prio) {
    ESSPrototype::setSpeed(s, 0, prio);
}

void ESSPrototype::setSpeed(char s, char a, bool prio) {
    int data = s || (a<<8) || (prio<<24);
    shm_mo.Lock();
    buf_mo.write(data);
    shm_mo.UnLock();
}

void ESSPrototype::setDirection(char d, bool prio) {
    int data = d || (prio<<24);
    std::cout << "########## ESSPROTO Attempting to lock!!!!!!!!!!!!!!!" << std::endl;
    shm_st.Lock();
    std::cout << " ########## ESSPROTOTYPE GOT THE LOCK!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    buf_st.write(data);
    std::cout << " ########## ESSPROTOTYPE WROTE THE DATA!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    shm_st.UnLock();
    std::cout << "########## ESSPROTO RELEASED THE LOCK!" << std::endl;
}

void ESSPrototype::setMinDistance(char d, bool prio) {
    int data = d || (prio<<24);
    shm_setmin.Lock();
    buf_setmin.write(data);
    shm_setmin.UnLock();
}

char ESSPrototype::getSpeed() {
    std::vector<int> data;
    shm_sp.Lock();
    int unreadValues = buf_sp.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_sp.read());
    shm_sp.UnLock();
    return (char) data.back(); // TODO make sure last element is the latest
}

char ESSPrototype::getDistance() {
    std::vector<int> data;
    shm_di.Lock();
    int unreadValues = buf_di.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(buf_di.read());
    shm_di.UnLock();

    int latest_i = data.back();
    char *latest = (char*) &latest_i; //TODO does this work?
    if (latest[0] < latest[1] && latest[0] < latest[2])
        return (char) data[0];
    else if (latest[1] < latest[0] && latest[1] < latest[2])
        return (char) data[1];
    else // latest[2] is smallest
        return (char) data[2];
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
    checkMotor();
    checkSpeedSensor();
    checkSteering();
    checkDistanceSensor();
    // TODO check camera
    return service_status_ == 0x00001111; // TODO take camera into account
}
