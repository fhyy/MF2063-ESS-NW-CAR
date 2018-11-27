#include "motor_speed_service.hpp"

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
MotorSpeedService::MotorSpeedService(uint32_t sp_sleep, uint8_t min_dist, bool skip_go) :
    run_(false),
    go_(false),
    use_dist_(false),
    shm_sp(CSharedMemory("/shm_sp")),
    shm_mo(CSharedMemory("/shm_mo")),
    pub_sp_sleep_(sp_sleep),
    min_dist_(min_dist),
    skip_go_(skip_go)
{
    int *p;

    shm_sp.Create(BUFFER_SIZE, O_RDWR);
    shm_sp.Attach(PROT_WRITE);
    p = (int*) shm_sp.GetData();
    buf_sp = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    sleep(2);

    shm_mo.Create(BUFFER_SIZE, O_RDWR);
    shm_mo.Attach(PROT_WRITE);
    p = (int*) shm_mo.GetData();
    buf_mo = Buffer(BUFFER_SIZE, p, B_PRODUCER);

}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- init ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool MotorSpeedService::init() {
    std::lock_guard<std::mutex> run_lock(mu_run_);

    pub_sp_thread_ = std::thread(std::bind(&MotorSpeedService::run_sp, this));

    app_ = vsomeip::runtime::get()->create_application("motor_speed_service");
    if (!app_->init()) {
        std::cerr << "Couldn't initialize application" << std::endl;
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();

    app_->register_state_handler(
        std::bind(&MotorSpeedService::on_state,
                  this,
                  std::placeholders::_1));

    app_->register_availability_handler(
        GO_SERVICE_ID,
        GO_INSTANCE_ID,
        std::bind(&MotorSpeedService::on_go_availability,
                  this,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    app_->register_availability_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        std::bind(&MotorSpeedService::on_dist_availability,
                  this,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    app_->register_message_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        DIST_EVENT_ID,
        std::bind(&MotorSpeedService::on_dist_eve,
                  this,
                  std::placeholders::_1));

    app_->register_message_handler(
        MOTOR_SERVICE_ID,
        MOTOR_INSTANCE_ID,
        MOTOR_METHOD_ID,
        std::bind(&MotorSpeedService::on_motor_req,
                  this,
                  std::placeholders::_1));

    app_->register_message_handler(
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        SHUTDOWN_METHOD_ID,
        std::bind(&MotorSpeedService::on_shutdown, this,
                  std::placeholders::_1));

    run_ = true;
    cond_run_.notify_all();

    return true;
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- start ---------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::start() {
    app_->start();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- stop ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::stop() {
    app_->stop();
    run_ = false;
    go_ = false;
    skip_go_ = false;
    pub_sp_thread_.join();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- is_running ----------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool MotorSpeedService::is_running() {
    return run_;
}

/*
 *#################################################################################################
 *################################# HERE BE PRIVATE MEMBER FUNCTIONS ##############################
 *#################################################################################################
 */

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_state ------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        app_->offer_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> embreak_group;
        embreak_group.insert(EMERGENCY_BREAK_EVENTGROUP_ID);
        app_->offer_event(MOTOR_SERVICE_ID,
                          MOTOR_INSTANCE_ID,
                          EMERGENCY_BREAK_EVENT_ID,
                          embreak_group,
                          true); //TODO what does this boolean do?

        app_->offer_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> speed_group;
        speed_group.insert(SPEED_EVENTGROUP_ID);
        app_->offer_event(SPEED_SERVICE_ID,
                          SPEED_INSTANCE_ID,
                          SPEED_EVENT_ID,
                          speed_group,
                          true); //TODO what does this boolean do?

        app_->request_service(GO_SERVICE_ID, GO_INSTANCE_ID);

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
    }
    else if (state == vsomeip::state_type_e::ST_REGISTERED) {
        app_->stop_offer_event(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENT_ID);
        app_->stop_offer_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);

        app_->stop_offer_event(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, EMERGENCY_BREAK_EVENT_ID);
        app_->stop_offer_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);

        app_->release_service(GO_SERVICE_ID, GO_INSTANCE_ID);

        app_->unsubscribe(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENTGROUP_ID);
        app_->release_event(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENT_ID);
        app_->release_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_dist_eve ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_dist_eve(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    std::cout << "DIST EVENT!!!!!!!!!! Data is: (" << (int) data[0] << ", " << (int) data[1]
              << ", " << (int) data[2] << ")" << std::endl;

    // If any sensor value falls below distance threshold create and send an embreak packet
    if (data[0]<min_dist_ || data[1]<min_dist_ || data[2]<min_dist_) {
        std::shared_ptr<vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();
        std::vector<vsomeip::byte_t> data;
        data.push_back(13);  // TODO empty payload?
        payload->set_data(data);
	    app_->notify(MOTOR_SERVICE_ID,
                        MOTOR_INSTANCE_ID,
                        EMERGENCY_BREAK_EVENT_ID,
                        payload,
                        true, true);
	std::cout << "EMBREAK EVENT SENT!!!!!" << std::endl;
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_motor_req --------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_motor_req(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    // Turn the four-element data packet into an int before writing
    int req = (data[3] << 24) || (data[2] << 16) || (data[1] << 8) || (data[0]);

    // Write to shared memory
    shm_mo.Lock();
    buf_mo.write(req);
    shm_mo.UnLock();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_setmin_req -------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_setmin_req(const std::shared_ptr<vsomeip::message> &msg) {
    // first element of a setmin packet will always be what we are looking for
    min_dist_ = msg->get_payload()->get_data()[0];
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_shutdown ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_shutdown(const std::shared_ptr<vsomeip::message>& msg) {
    stop();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_go_availability --------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_go_availability(vsomeip::service_t serv,
                                          vsomeip::instance_t inst,
                                          bool go) {
    if (GO_SERVICE_ID == serv && GO_INSTANCE_ID == inst) {
        if (go_ && !go) {
            go_ = false;
        }
        else if (!go_ && go) {
            go_ = true;
	    std::cout << "GOOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!" << std::endl;
        }
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_dist_availability ------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_dist_availability(vsomeip::service_t serv,
                                          vsomeip::instance_t inst,
                                          bool use_dist) {
    if (DIST_SERVICE_ID == serv && DIST_INSTANCE_ID == inst) {
        if (use_dist_ && !use_dist) {
            use_dist_ = false;
        }
        else if (!use_dist_ && use_dist) {
            use_dist_ = true;
        }
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- run_sp --------------------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::run_sp() {
    { // Synchronization lock upon initialization
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    // Thread loop
    while(run_) {

        // Pause here if !go_
        while(!(go_ || skip_go_));

        // Store values from shared memory in sensor_data
        std::vector<int> sensor_data;
        shm_sp.Lock();
        int unreadValues = buf_sp.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            sensor_data.push_back(buf_sp.read());
        shm_sp.UnLock();

        // prepare and send transmission if data was read from shared memory
        if (sensor_data.size() > 0) {

            
            // Use only newest sensor value for transmission
            int sensor_data_latest = sensor_data.back();

            // turn int into std::vector of four vsomeip::byte_t
            std::vector<vsomeip::byte_t> sensor_data_formatted;
            char byte;
            for (int j=0; j<4; j++) {
                // first element of of vector is lowest 8 bits and so on
                byte = (sensor_data_latest >> j*8);
                sensor_data_formatted.push_back(byte);
            }

            // Priority (0x0000=low, other=high) could be set here in a future implementation
            // sensor_data_formatted[3] = priority;

            // set data and publish it on the network TODO protect app with mutex
		    payload_->set_data(sensor_data_formatted);
            app_->notify(SPEED_SERVICE_ID, SPEED_INSTANCE_ID,
                         SPEED_EVENT_ID, payload_, true, true);
    	    std::cout << "SPEED EVENT SENT! Data: (" << sensor_data_formatted[0] << ", "
                      << sensor_data_formatted[1] << ", " << sensor_data_formatted[2] << ", "
                      << sensor_data_formatted[3] << ")" << std::endl;
        }

        //sleep before repeating the thread loop
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_sp_sleep_));
    }
}

/*
 *#################################################################################################
 *################################# MAIN ##########################################################
 *#################################################################################################
 */

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    MotorSpeedService *mss_ptr(nullptr);

    void handle_signal(int signal) {
        std::cout << "Interrupt signal: " << signal << std::endl;
        if (mss_ptr != nullptr &&
                (signal == SIGINT || signal == SIGTERM))
            mss_ptr->stop();
    }
#endif

int main(int argc, char** argv) {
    // TODO start by sleeping, let producer get a head start

    uint32_t sp_sleep = 30;
    uint8_t min_dist = 100;
    bool skip_go = false;

    std::string sleep_flag("--sleep");
    std::string min_dist_flag("--min-dist");
    std::string skip_go_flag("--skip-go");

    for (int i=1; i<argc; i++) {
        if (sleep_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> sp_sleep;
        }
        else if (min_dist_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> min_dist;
        }
        else if (skip_go_flag==argv[i]) {
            skip_go = true;
        }
    }

    MotorSpeedService mss(sp_sleep, min_dist, skip_go);

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    mss_ptr = &mss;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
#endif

    if (mss.init()) {
        mss.start();
        return 0;
    }
    else
        return 1;
}
