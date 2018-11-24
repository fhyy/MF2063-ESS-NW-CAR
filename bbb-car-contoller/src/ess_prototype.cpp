#include "ess_prototype.hpp"

ESSPrototype::ESSPrototype() {
    int *p;

    CSharedMemory shmMemory_in("/testSharedmemory3");
    shmMemory_sp.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_sp.Attach(PROT_WRITE);
    p = (int*) shmMemory_sp.GetData();
    Buffer circBuffer_sp(BUFFER_SIZE, p, B_CONSUMER);

    CSharedMemory shmMemory_di("/testSharedmemory4");
    shmMemory_di.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_di.Attach(PROT_WRITE);
    p = (int*) shmMemory_di.GetData();
    Buffer circBuffer_di(BUFFER_SIZE, p, B_CONSUMER);

    CSharedMemory shmMemory_go("/testSharedmemory5");
    shmMemory_go.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_go.Attach(PROT_WRITE);
    p = (int*) shmMemory_go.GetData();
    Buffer circBuffer_go(BUFFER_SIZE, p, B_CONSUMER);

    CSharedMemory shmMemory_mo("/testSharedmemory1");
    shmMemory_mo.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_mo.Attach(PROT_WRITE);
    p = (int*) shmMemory_mo.GetData();
    Buffer circBuffer_mo(BUFFER_SIZE, p, B_PRODUCER);

    CSharedMemory shmMemory_st("/testSharedmemory2");
    shmMemory_st.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_st.Attach(PROT_WRITE);
    p = (int*) shmMemory_st.GetData();
    Buffer circBuffer_st(BUFFER_SIZE, p, B_PRODUCER);

    shmMemory_setmin = CSharedMemory("/testSharedmemory6");
    shmMemory_setmin.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_setmin.Attach(PROT_WRITE);
    p = (int*) shmMemory_setmin.GetData();
    circBuffer_setmin = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shmMemory_cam = CSharedMemory("/testSharedmemory7");
    shmMemory_cam.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_cam.Attach(PROT_WRITE);
    p = (int*) shmMemory_cam.GetData();
    circBuffer_cam = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shmMemory_shutdown = CSharedMemory("/testSharedmemory8");
    shmMemory_shutdown.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_shutdown.Attach(PROT_WRITE);
    p = (int*) shmMemory_shutdown.GetData();
    circBuffer_shutdown = Buffer(BUFFER_SIZE, p, B_PRODUCER);
}

//TODO bool EssPrototype::boot() {}

void ESSPrototype::shutdown() {
    int data = 1;
    shmMemory_shutdown.Lock();
    circBuffer_shutdown.write(data);
    shmMemory_shutdown.UnLock();
}

bool ESSPrototype::checkMotor() {
    int mo_mask = 0x00000001;
    std::vector<int> data;

    shmMemory_go.Lock();
    int unreadValues = circBuffer_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_go.read());
    shmMemory_go.UnLock();

    service_status_ = data.back();

    return service_status_ && mo_mask;
    //TODO make sure last element is the latest
}

bool ESSPrototype::checkSpeedSensor() {
    int sp_mask = 0x00000100;
    std::vector<int> data;

    shmMemory_go.Lock();
    int unreadValues = circBuffer_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_go.read());
    shmMemory_go.UnLock();

    service_status_ = data.back();

    return service_status_ && sp_mask;
    //TODO make sure last element is the latest
}

bool ESSPrototype::checkSteering() {
    int st_mask = 0x00000010;
    std::vector<int> data;

    shmMemory_go.Lock();
    int unreadValues = circBuffer_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_go.read());
    shmMemory_go.UnLock();

    service_status_ = data.back();

    return service_status_ && st_mask;
    //TODO make sure last element is the latest
}

bool ESSPrototype::checkDistanceSensor() {
    int di_mask = 0x00001000;
    std::vector<int> data;

    shmMemory_go.Lock();
    int unreadValues = circBuffer_go.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_go.read());
    shmMemory_go.UnLock();

    service_status_ = data.back();

    return service_status_ && di_mask;
    //TODO make sure last element is the latest
}

void ESSPrototype::setSpeed(char s, bool prio) {
    ESSPrototype::setSpeed(s, 0, prio);
}

void ESSPrototype::setSpeed(char s, char a, bool prio) {
    int data = s || (a<<8) || (prio<<24);
    shmMemory_di.Lock();
    circBuffer_di.write(data);
    shmMemory_di.UnLock();
}

void ESSPrototype::setDirection(char d, bool prio) {
    int data = d || (prio<<24);
    shmMemory_di.Lock();
    circBuffer_di.write(data);
    shmMemory_di.UnLock();
}

void ESSPrototype::setMinDistance(char d, bool prio) {
    int data = d || (prio<<24);
    shmMemory_setmin.Lock();
    circBuffer_setmin.write(data);
    shmMemory_setmin.UnLock();
}

char ESSPrototype::getSpeed() {
    std::vector<int> data;
    shmMemory_sp.Lock();
    int unreadValues = circBuffer_sp.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_sp.read());
    shmMemory_sp.UnLock();
    return (char) data.back(); // TODO make sure last element is the latest
}

char ESSPrototype::getDistance() {
    std::vector<int> data;
    shmMemory_di.Lock();
    int unreadValues = circBuffer_di.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_di.read());
    shmMemory_di.UnLock();

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
    shmMemory_di.Lock();
    int unreadValues = circBuffer_di.getUnreadValues();
    for (int i=0; i<unreadValues; i++)
        data.push_back(circBuffer_di.read());
    shmMemory_di.UnLock();

    int latest = data.back();

    TODO decode latest and create flag object

    return flag;
}*/

bool ESSPrototype::getGoStatus() {
    return service_status_ == 0x00001111;
}
