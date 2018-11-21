#include "motor_speed_service.hpp"

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
MotorSpeedService::MotorSpeedService(uint32_t sp_sleep, uint8_t min_dist) :
    run_(false),
    go_(false),
    use_dist_(false),
    pub_sp_sleep_(sp_sleep),
    min_dist_(min_dist)
    {}

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
 *--------------------------------- on_dist_eve --------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_dist_eve(const std::shared_ptr<vsomeip::message> &msg) {
    //TODO if dist<min_dist_
    //            app_->notify();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_motor_req --------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_motor_req(const std::shared_ptr<vsomeip::message> &msg) {
    // TODO unpack message and write to shared variable
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_setmin_req -------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_setmin_req(const std::shared_ptr<vsomeip::message> &msg) {
    min_dist_ = *(msg->get_payload()->get_data());
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
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    while(run_) {
        while(!go_);

        // TODO Replace this block with geting arduio values -------------------------------------
        std::vector<vsomeip::byte_t> data;
        data.push_back(7);
        data.push_back(11);
        data.push_back(23);
        payload_->set_data(data);
        //------------------------------------------------------------------
        
        app_->notify(SPEED_SERVICE_ID, SPEED_INSTANCE_ID,
                     SPEED_EVENT_ID, payload_, true, true);

        std::this_thread::sleep_for(std::chrono::milliseconds(pub_sp_sleep_));
    }
}











































