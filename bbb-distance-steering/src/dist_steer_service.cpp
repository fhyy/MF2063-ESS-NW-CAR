#include "dist_steer_service.hpp"

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
DistSteerService::DistSteerService(uint32_t di_sleep) :
    run_(false),
    go_(false),
    pub_di_sleep_(di_sleep)
    {}

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
    // TODO unpack message and write to shared variable
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
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    while(run_) {
        while(!go_);

        // TODO Replace this block with geting arduio values -------------------------------------
        std::vector<vsomeip::byte_t> data;
        data.push_back(122);
        data.push_back(133);
        data.push_back(130);
        payload_->set_data(data);
        //------------------------------------------------------------------

        app_->notify(DIST_SERVICE_ID, DIST_INSTANCE_ID,
                     DIST_EVENT_ID, payload_, true, true);
	std::cout << "DIST EVENT SENT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_di_sleep_));
    }
}
