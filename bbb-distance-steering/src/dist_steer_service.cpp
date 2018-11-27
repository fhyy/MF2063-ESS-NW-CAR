#include "dist_steer_service.hpp"

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
DistSteerService::DistSteerService(uint32_t di_sleep, bool skip_go) :
    run_(false),
    go_(false),
    shm_di(CSharedMemory("/shm_di")),
    shm_st(CSharedMemory("/shm_st")),
    skip_go_(skip_go),
    pub_di_sleep_(di_sleep)
{
    int *p;

    shm_di.Create(BUFFER_SIZE, O_RDWR);
    shm_di.Attach(PROT_WRITE);
    p = (int*) shm_di.GetData();
    buf_di = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    sleep(2);

    shm_st.Create(BUFFER_SIZE, O_RDWR);
    shm_st.Attach(PROT_WRITE);
    p = (int*) shm_st.GetData();
    buf_st = Buffer(BUFFER_SIZE, p, B_PRODUCER);
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- init ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool DistSteerService::init() {
    std::lock_guard<std::mutex> run_lock(mu_run_);

    pub_di_thread_ = std::thread(std::bind(&DistSteerService::run_di, this));

    app_ = vsomeip::runtime::get()->create_application("dist_steer_service");
    if (!app_->init()) {
        std::cerr << "Couldn't initialize application" << std::endl;
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();

    app_->register_state_handler(
        std::bind(&DistSteerService::on_state, this,
        std::placeholders::_1));

    app_->register_availability_handler(GO_SERVICE_ID, GO_INSTANCE_ID,
        std::bind(&DistSteerService::on_go_availability,
        this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    app_->register_message_handler(
        STEER_SERVICE_ID,
        STEER_INSTANCE_ID,
        STEER_METHOD_ID,
        std::bind(&DistSteerService::on_steer_req, this,
                  std::placeholders::_1));

    app_->register_message_handler(
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        SHUTDOWN_METHOD_ID,
        std::bind(&DistSteerService::on_shutdown, this,
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
void DistSteerService::start() {
    app_->start();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- stop ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void DistSteerService::stop() {
    app_->stop();
    run_ = false;
    go_ = false;
    skip_go_ = false;
    pub_di_thread_.join();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- is_running ----------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool DistSteerService::is_running() {
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
void DistSteerService::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        app_->offer_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);

        app_->offer_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> dist_group;
        dist_group.insert(DIST_EVENTGROUP_ID);
        app_->offer_event(DIST_SERVICE_ID,
                          DIST_INSTANCE_ID,
                          DIST_EVENT_ID,
                          dist_group,
                          true); //TODO what does this boolean do?

        app_->request_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    else if (state == vsomeip::state_type_e::ST_DEREGISTERED) {
        app_->stop_offer_event(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENT_ID);
        app_->stop_offer_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);

        app_->stop_offer_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);

        app_->release_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_steer_req --------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void DistSteerService::on_steer_req(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    // Turn the four-element data packet into an int before writing
    int req = (data[3] << 24) || (data[2] << 16) || (data[1] << 8) || (data[0]);
    std::cout << "Got steer request!! Data: " << req << std::endl;
    // Write to shared memory
    shm_st.Lock();
    buf_st.write(req);
    shm_st.UnLock();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_shutdown ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void DistSteerService::on_shutdown(const std::shared_ptr<vsomeip::message>& msg) {
    stop();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_go_availability --------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void DistSteerService::on_go_availability(vsomeip::service_t serv,
                                          vsomeip::instance_t inst,
                                          bool go) {
    if (GO_SERVICE_ID == serv && GO_INSTANCE_ID == inst) {
        if (go_ && !go) {
            go_ = false;
        }
        else if (!go_ && go) {
	    std::cout << "GOOOOOOOOOOO!!!!!!!!!!!!!!!!" << std::endl;
            go_ = true;
        }
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- run_di --------------------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void DistSteerService::run_di() {
    // Synchronization lock upon initialization
    {
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

        shm_di.Lock(); //Lock the shared memory
        int unreadValues = buf_di.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            sensor_data.push_back(buf_di.read());
        shm_di.UnLock(); //Unlock the shared memory

        // prepare and send transmission if data was read from shared memory
        if (sensor_data.size() > 0) {

            // Use only newest sensor values for transmission
            int sensor_data_latest = sensor_data.back();

            // turn int into std::vector of four vsomeip::byte_t
            std::vector<vsomeip::byte_t> sensor_data_formatted;
            char byte;
            for (int j=0; j<4; j++) {
                // first element of of vector is lowest 8 bits and so on
                byte = (sensor_data_latest >> j*8);
                sensor_data_formatted.push_back(byte);

                // Priority (0x0000=low, other=high) could be set here in a future implementation
                // sensor_data_formatted[3] = priority;
            }

            // set data and publish it on the network TODO protect app with mutex
            payload_->set_data(sensor_data_formatted);
            app_->notify(DIST_SERVICE_ID, DIST_INSTANCE_ID,
                         DIST_EVENT_ID, payload_, true, true);
    	    std::cout << "DIST EVENT SENT! Data: (" << sensor_data_formatted[0] << ", "
                      << sensor_data_formatted[1] << ", " << sensor_data_formatted[2] << ", "
                      << sensor_data_formatted[4] << ")" << std::endl;
        }

        //sleep before repeating the thread loop
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_di_sleep_));
    }
}

/*
 *#################################################################################################
 *################################# MAIN ##########################################################
 *#################################################################################################
 */

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    DistSteerService *dss_ptr(nullptr);

    void handle_signal(int signal) {
        std::cout << "Interrupt signal: " << signal << std::endl;
        if (dss_ptr != nullptr &&
                (signal == SIGINT || signal == SIGTERM))
            dss_ptr->stop();
    }
#endif

int main(int argc, char** argv) {
    uint32_t di_sleep = 3000;
    bool skip_go;

    std::string sleep_flag("--sleep");
    std::string skip_go_flag("--skip-go"); // should only be used for testing and debugging

    for (int i=1; i<argc; i++) {
        if (sleep_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> di_sleep;
        }
        else if (skip_go_flag==argv[i]) {
            skip_go = true;
        }
    }

    DistSteerService dss(di_sleep, skip_go);

#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    dss_ptr = &dss;
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
#endif

    if (dss.init()) {
        dss.start();
        return 0;
    }
    else
        return 1;
}
