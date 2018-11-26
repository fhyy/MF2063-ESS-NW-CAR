#include "car_ctrl_client.hpp"

CarCTRLClient::CarCTRLClient(uint32_t mo_sleep, uint32_t st_sleep, uint32_t setmin_sleep) :
    run_(false),
    go_(false),
    is_ava_di_(false),
    is_ava_st_(false),
    is_ava_mo_(false),
    is_ava_sp_(false),
    is_ava_cam_(false),
    app_busy_(false),
    req_mo_sleep_(mo_sleep),
    req_st_sleep_(st_sleep),
    req_setmin_sleep_(setmin_sleep),
    req_shutdown_sleep_(103) // TODO make settable through CTR?
{
    int *p;

    shmMemory_mo = CSharedMemory("/shm_motor_cons");
    shmMemory_mo.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_mo.Attach(PROT_WRITE);
    p = (int*) shmMemory_mo.GetData();
    circBuffer_mo = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shmMemory_st = CSharedMemory("/shm_steer_cons");
    shmMemory_st.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_st.Attach(PROT_WRITE);
    p = (int*) shmMemory_st.GetData();
    circBuffer_st = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shmMemory_setmin = CSharedMemory("/shm_setmin_cons");
    shmMemory_setmin.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_setmin.Attach(PROT_WRITE);
    p = (int*) shmMemory_setmin.GetData();
    circBuffer_setmin = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shmMemory_shutdown = CSharedMemory("/shm_shutdown_cons");
    shmMemory_shutdown.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_shutdown.Attach(PROT_WRITE);
    p = (int*) shmMemory_shutdown.GetData();
    circBuffer_shutdown = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    // sleep while other process finishes consumer init

    shmMemory_sp = CSharedMemory("/shm_speed_prod");
    shmMemory_sp.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_sp.Attach(PROT_WRITE);
    p = (int*) shmMemory_sp.GetData();
    circBuffer_sp = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shmMemory_di = CSharedMemory("/shm_dist_prod");
    shmMemory_di.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_di.Attach(PROT_WRITE);
    p = (int*) shmMemory_di.GetData();
    circBuffer_di = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shmMemory_go = CSharedMemory("/shm_go_prod");
    shmMemory_go.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_go.Attach(PROT_WRITE);
    p = (int*) shmMemory_go.GetData();
    circBuffer_go = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shmMemory_cam = CSharedMemory("/shm_cam_prod");
    shmMemory_cam.Create(BUFFER_SIZE, O_RDWR);
    shmMemory_cam.Attach(PROT_WRITE);
    p = (int*) shmMemory_cam.GetData();
    circBuffer_cam = Buffer(BUFFER_SIZE, p, B_PRODUCER);
}

bool CarCTRLClient::init() {
    std::lock_guard<std::mutex> run_lock(mu_run_);

    req_mo_thread_ = std::thread(std::bind(&CarCTRLClient::send_motor_req, this));
    req_st_thread_ = std::thread(std::bind(&CarCTRLClient::send_steer_req, this));
    req_setmin_thread_ = std::thread(std::bind(&CarCTRLClient::send_setmin_req, this));
    req_shutdown_thread_ = std::thread(std::bind(&CarCTRLClient::send_shutdown_req, this));

    app_ = vsomeip::runtime::get()->create_application("car_ctrl_client");
    if (!app_->init()) {
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();
    request_ = vsomeip::runtime::get()->create_request(false); // false=>UDP

    app_->register_state_handler(
        std::bind(&CarCTRLClient::on_state, this,
        std::placeholders::_1));

    app_->register_availability_handler(vsomeip::ANY_SERVICE,
                                        vsomeip::ANY_INSTANCE,
                                        std::bind(&CarCTRLClient::on_availability,
                                                  this,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2,
                                                  std::placeholders::_3));

    app_->register_message_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        DIST_EVENT_ID,
        std::bind(&CarCTRLClient::on_dist_eve, this,
                  std::placeholders::_1));

    app_->register_message_handler(
        SPEED_SERVICE_ID,
        SPEED_INSTANCE_ID,
        SPEED_EVENT_ID,
        std::bind(&CarCTRLClient::on_speed_eve, this,
                  std::placeholders::_1));

    app_->register_message_handler(
        CAM_SERVICE_ID,
        CAM_INSTANCE_ID,
        CAM_EVENT_ID,
        std::bind(&CarCTRLClient::on_cam_eve, this,
                  std::placeholders::_1));

    app_->register_message_handler(
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        EMERGENCY_BREAK_EVENT_ID,
        std::bind(&CarCTRLClient::on_embreak_eve, this,
                  std::placeholders::_1));

    run_ = true;
    cond_run_.notify_all();

    return true;
}

void CarCTRLClient::start() {
    app_->start();

}

void CarCTRLClient::stop() {
    req_mo_thread_.join();
    req_st_thread_.join();
    req_setmin_thread_.join();
    req_shutdown_thread_.join();
    go_ = false;
    app_->stop();
}

/*
 *-------------------------------------------------------------------------------------------------
 *                                  HERE BE PRIVATE MEMBER FUNCTIONS
 *-------------------------------------------------------------------------------------------------
 */

void CarCTRLClient::update_go_status() {
    if (!run_)
        return;
    else if (is_ava_di_ && is_ava_st_ && is_ava_mo_ && is_ava_sp_) {
        go_ = true;
        app_->offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    else {
        go_ = false;
        app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }

    // error here
    int mo_onehot = 0x00000001;
    int st_onehot = 0x00000010;
    int sp_onehot = 0x00000100;
    int di_onehot = 0x00001000;
    int service_status = mo_onehot && is_ava_mo_ ||
                         st_onehot && is_ava_st_ ||
                         sp_onehot && is_ava_sp_ ||
                         di_onehot && is_ava_mo_; // TODO add camera

    // TODO fix error caused by these lines
    /*shmMemory_go.Lock();
    circBuffer_go.write(service_status);
    shmMemory_go.UnLock();*/
}

/*
 * This is a thread!
 */
void CarCTRLClient::send_motor_req() {
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    while(run_) {
        while(!go_);

        std::vector<vsomeip::byte_t> req_data;

        shmMemory_mo.Lock();
        int unreadValues = circBuffer_mo.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back((vsomeip::byte_t) circBuffer_mo.read());
        shmMemory_mo.UnLock();

        if (req_data.size() > 0) {
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            send_req(req_data, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, MOTOR_METHOD_ID);

            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(req_mo_sleep_));
    }
}

/*
 * This is a thread!
 */
void CarCTRLClient::send_steer_req() {
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    while(run_) {
        while(!go_);

        std::vector<vsomeip::byte_t> req_data;

        shmMemory_st.Lock();
        int unreadValues = circBuffer_st.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back((vsomeip::byte_t) circBuffer_st.read());
        shmMemory_st.UnLock();

        if (req_data.size() > 0) {
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            send_req(req_data, STEER_SERVICE_ID, STEER_INSTANCE_ID, STEER_METHOD_ID);

            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(req_st_sleep_));
    }
}

/*
 * This is a thread!
 */
void CarCTRLClient::send_setmin_req() {
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    while(run_) {
        while(!go_);

        std::vector<vsomeip::byte_t> req_data;

        shmMemory_setmin.Lock();
        int unreadValues = circBuffer_setmin.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back((vsomeip::byte_t) circBuffer_setmin.read());
        shmMemory_setmin.UnLock();

        if (req_data.size() > 0) {
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            send_req(req_data, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, SETMIN_METHOD_ID);

            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(req_setmin_sleep_));
    }
}

/*
 * This is a thread!
 */
void CarCTRLClient::send_shutdown_req() {
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    while(run_) {
        while(!go_);

        std::vector<vsomeip::byte_t> req_data;

        shmMemory_shutdown.Lock();
        int unreadValues = circBuffer_shutdown.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back((vsomeip::byte_t) circBuffer_shutdown.read());
        shmMemory_shutdown.UnLock();

        if (req_data.size() > 0 && req_data.back() == 1) {
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            send_req(req_data, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(req_data, STEER_SERVICE_ID, STEER_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(req_data, SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(req_data, DIST_SERVICE_ID, DIST_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(req_data, CAM_SERVICE_ID, CAM_INSTANCE_ID, SHUTDOWN_METHOD_ID);

            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(req_shutdown_sleep_));
    }
}

void CarCTRLClient::on_dist_eve(const std::shared_ptr<vsomeip::message> &msg) {
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    vsomeip::length_t datalength = msg->get_payload()->get_length();

    shmMemory_di.Lock(); // TODO ask Jacob if it is okay to lock outside for-loop
    for(int i=0; i<datalength; i++) {
        circBuffer_di.write(data[i]);
    }
    shmMemory_di.UnLock();
}

void CarCTRLClient::on_speed_eve(const std::shared_ptr<vsomeip::message> &msg) {
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    vsomeip::length_t datalength = msg->get_payload()->get_length();

    shmMemory_sp.Lock(); // TODO ask Jacob if it is okay to lock outside for-loop
    for(int i=0; i<datalength; i++) {
        circBuffer_sp.write(data[i]);
    }
    shmMemory_sp.UnLock();
}

void CarCTRLClient::on_cam_eve(const std::shared_ptr<vsomeip::message> &msg) {
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    vsomeip::length_t datalength = msg->get_payload()->get_length();

    shmMemory_cam.Lock(); // TODO ask Jacob if it is okay to lock outside for-loop
    for(int i=0; i<datalength; i++) {
        circBuffer_cam.write(data[i]);
    }
    shmMemory_cam.UnLock();
}

void CarCTRLClient::on_embreak_eve(const std::shared_ptr<vsomeip::message> &msg) {
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    std::cout << "EMBREAK EVENT!!!!!!!!!! Data is: " << (int) data[0]  << std::endl;
}

void CarCTRLClient::send_req(std::vector<vsomeip::byte_t> data,
                             vsomeip::service_t serv,
                             vsomeip::instance_t inst,
                             vsomeip::method_t meth) {

    request_->set_service(serv);
    request_->set_instance(inst);
    request_->set_method(meth);

    payload_->set_data(data);
    request_->set_payload(payload_);
    app_->send(request_, true);
}

void CarCTRLClient::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        app_->request_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> dist_group;
        dist_group.insert(DIST_EVENTGROUP_ID);
        app_->request_event(
                DIST_SERVICE_ID,
                DIST_INSTANCE_ID,
                DIST_EVENT_ID,
                dist_group,
                false); // TODO what does this boolean do?
        app_->subscribe(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENTGROUP_ID);

        app_->request_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);

        app_->request_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> speed_group;
        speed_group.insert(SPEED_EVENTGROUP_ID);
        app_->request_event(
                SPEED_SERVICE_ID,
                SPEED_INSTANCE_ID,
                SPEED_EVENT_ID,
                speed_group,
                false); // TODO what does this boolean do?
        app_->subscribe(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENTGROUP_ID);

        app_->request_service(CAM_SERVICE_ID, CAM_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> cam_group;
        cam_group.insert(CAM_EVENTGROUP_ID);
        app_->request_event(
                CAM_SERVICE_ID,
                CAM_INSTANCE_ID,
                CAM_EVENT_ID,
                cam_group,
                false); // TODO what does this boolean do?
        app_->subscribe(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENTGROUP_ID);

        app_->request_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> embreak_group;
        embreak_group.insert(EMERGENCY_BREAK_EVENTGROUP_ID);
        app_->request_event(
                MOTOR_SERVICE_ID,
                MOTOR_INSTANCE_ID,
                EMERGENCY_BREAK_EVENT_ID,
                embreak_group,
                false); // TODO what does this boolean do?
        app_->subscribe(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, EMERGENCY_BREAK_EVENTGROUP_ID);
    }
    else if(state == vsomeip::state_type_e::ST_DEREGISTERED) {
        app_->release_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);

        app_->unsubscribe(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENTGROUP_ID);
        app_->release_event(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENT_ID);
        app_->release_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);

        app_->unsubscribe(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENTGROUP_ID);
        app_->release_event(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENT_ID);
        app_->release_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);

        app_->unsubscribe(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, EMERGENCY_BREAK_EVENTGROUP_ID);
        app_->release_event(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, EMERGENCY_BREAK_EVENT_ID);
        app_->release_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);

        app_->unsubscribe(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENTGROUP_ID);
        app_->release_event(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENT_ID);
        app_->release_service(CAM_SERVICE_ID, CAM_INSTANCE_ID);

        if(go_)
            go_ = false;
            app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
}

void CarCTRLClient::on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool is_ava) {
    if (DIST_SERVICE_ID == serv && DIST_INSTANCE_ID == inst) {
        if (is_ava_di_ && !is_ava) {
            is_ava_di_ = false;
        }
        else if (!is_ava_di_ && is_ava) {
            is_ava_di_ = true;
        }
    }
    else if (STEER_SERVICE_ID == serv && STEER_INSTANCE_ID == inst) {
        if (is_ava_st_ && !is_ava) {
            is_ava_st_ = false;
        }
        else if (!is_ava_st_ && is_ava) {
            is_ava_st_ = true;
        }
    }
    else if (MOTOR_SERVICE_ID == serv && MOTOR_INSTANCE_ID == inst) {
        if (is_ava_mo_ && !is_ava) {
            is_ava_mo_ = false;
        }
        else if (!is_ava_mo_ && is_ava) {
            is_ava_mo_ = true;
        }
    }
    else if (SPEED_SERVICE_ID == serv && SPEED_INSTANCE_ID == inst) {
        if (is_ava_sp_ && !is_ava) {
            is_ava_sp_ = false;
        }
        else if (!is_ava_sp_ && is_ava) {
            is_ava_sp_ = true;
        }
    }
    else if (CAM_SERVICE_ID == serv && CAM_INSTANCE_ID == inst) {
        if (is_ava_cam_ && !is_ava) {
            is_ava_cam_ = false;
        }
        else if (!is_ava_cam_ && is_ava) {
            is_ava_cam_ = true;
        }
    }
    update_go_status();
}

/*
 *#################################################################################################
 *################################# MAIN ##########################################################
 *#################################################################################################
 */

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    CarCTRLClient *ccc_ptr(nullptr);

    void handle_signal(int signal) {
        std::cout << "Interrupt signal: " << signal << std::endl;
        if (ccc_ptr != nullptr &&
                (signal == SIGINT || signal == SIGTERM))
            ccc_ptr->stop();
    }
#endif

int main(int argc, char** argv) {

    uint32_t mo_sleep = 33;
    uint32_t st_sleep = 47;
    uint32_t setmin_sleep = 93;

    std::string motor_flag("--motor-sleep");
    std::string steer_flag("--steer-sleep");
    std::string setmin_flag("--setmin-sleep");

    for (int i=1; i<argc; i++) {
        if (steer_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> mo_sleep;
        }
        else if (motor_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> st_sleep;
        }
        else if (setmin_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> setmin_sleep;
        }
    }

    CarCTRLClient ccc(mo_sleep, st_sleep, setmin_sleep);

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    ccc_ptr = &ccc;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
#endif

    if (ccc.init()) {
        ccc.start();
        return 0;
    }
    else
        return 1;
}
