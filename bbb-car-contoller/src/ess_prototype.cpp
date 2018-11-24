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
}

//TODO bool EssPrototype::boot() {}

//TODO bool EssPrototype::shutdown() {}

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

bool ESSPrototype::setSpeed(char s, bool prio) {
    ESSPrototype::setSpeed(s, 0, prio);
}

bool ESSPrototype::setSpeed(char s, char a, bool prio) {
    int data = s || (a<<8) || (prio<<16);
    shmMemory_di.Lock();
    circBuffer_di.write(data);
    shmMemory_di.UnLock();
}

bool ESSPrototype::setDirection(char d, bool prio) {
    int data = d || (prio<<8);
    shmMemory_di.Lock();
    circBuffer_di.write(data);
    shmMemory_di.UnLock();
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
    return (char) data.back(); // TODO make sure last element is the latest
}

bool ESSPrototype::getGoStatus() {
    return service_status_ == 0x00001111;
}
