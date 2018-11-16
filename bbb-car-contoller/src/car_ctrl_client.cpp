#include "car_ctrl_client.hpp"

CarCTRLClient::CarCTRLClient(uint32_t st_sleep, uint32_t mo_sleep, uint32_t n) :
    req_st_sleep_(st_sleep),
    req_mo_sleep_(mo_sleep),
    slaves_tot_(n),
    slaves_disc_(0),
    is_init_(false),
    go_(false),
    is_ava_st_(false),
    is_ava_mo_(false),
    app_busy_(false)
    {}

bool CarCTRLClient::init() {
    std::lock_guard<std::mutex> init_lk(mu_init_);

    req_st_thread_ = std::thread(std::bind(&CarCTRLClient::run_st, this));
    req_mo_thread_ = std::thread(std::bind(&CarCTRLClient::run_mo, this));

    app_ = vsomeip::runtime::get()->create_application("car_ctr_client");
    if (!app_->init()) {
        std::cerr << "############### Couldn't initialize application" << std::endl;
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();
    request_ = vsomeip::runtime::get()->create_request(false); // false=>UDP


    app_->register_state_handler(
        std::bind(&CarCTRLClient::on_state, this,
        std::placeholders::_1));

    app_->register_availability_handler(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID,
        std::bind(&CarCTRLClient::on_availability,
        this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));


    app_->register_message_handler(
        DIST_STEER_SERVICE_ID,
        DIST_STEER_INSTANCE_ID,
        DIST_EVENT_ID,
        std::bind(&CarCTRLClient::on_dist_eve, this,
                  std::placeholders::_1));

    is_init_ = true;
    cond_init_.notify_all();

    std::cout << "############### Successfully initialized" << std::endl; // TODO remove

    return true;
}

void CarCTRLClient::start() {
    app_->start();
    std::cout << "############### Successfully started" << std::endl; // TODO remove
}

void CarCTRLClient::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        std::cout << "################ Succesfully registered at runtime!" << std::endl; // TODO remove

        app_->request_service(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID);
        std::cout << "################ Succesfully requested at dist/steer!" << std::endl; // TODO remove

        std::set<vsomeip::eventgroup_t> dist_group;
        dist_group.insert(DIST_EVENTGROUP_ID);
        app_->request_event(
                DIST_STEER_SERVICE_ID,
                DIST_STEER_INSTANCE_ID,
                DIST_EVENT_ID,
                dist_group,
                false); // TODO what does this boolean do?
        app_->subscribe(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID, DIST_EVENTGROUP_ID);

        std::cout << "################ Succesfully subscribed to dist!" << std::endl; // TODO remove

        // TODO similar request for motor server

        // TODO similar subscription to speed events

    }
    // TODO handle deregistration
}

void CarCTRLClient::on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool is_ava) {
                std::cout << "############### Go status: " << go_ << std::endl; // TODO remove
                std::cout << "############### Init status: " << is_init_ << std::endl; // TODO remove
                std::cout << "############### Slave status: " << slaves_disc_ << std::endl;
        if (DIST_STEER_SERVICE_ID == serv && DIST_STEER_INSTANCE_ID == inst) {
            if (is_ava_st_ && !is_ava) {
                is_ava_st_ = false;
                slaves_disc_--;
                check_go_status();
                std::cout << "############### STEERING NOT AVAILABLE" << std::endl; // TODO remove
                std::cout << "############### Go status: " << go_ << std::endl; // TODO remove
                std::cout << "############### Init status: " << is_init_ << std::endl; // TODO remove
                std::cout << "############### Slave status: " << slaves_disc_ << std::endl;
            } else if (!is_ava_st_ && is_ava) {
                is_ava_st_ = true;
                slaves_disc_++;
                check_go_status();
                std::cout << "############### STEERING AVAILABLE" << std::endl; // TODO remove
                std::cout << "############### Go status: " << go_ << std::endl; // TODO remove
                std::cout << "############### Init status: " << is_init_ << std::endl; // TODO remove
                std::cout << "############### Slave status: " << slaves_disc_ << std::endl;
            }
        }
        else {
            std::cout << "############### on_availability triggd, no match" << std::endl; // TODO remove
        }
}

void CarCTRLClient::on_dist_eve(const std::shared_ptr<vsomeip::message> &_msg) {
    std::cout << "############### dist event received" << std::endl; // TODO remove
    // TODO code for sharing payload with arduino system
}

// TODO on_availability and on_event for motor server

/*
 * Checks if the dexpected number of slaves have been discovered
 */
void CarCTRLClient::check_go_status() {
    std::cout << "############### checking go status" << std::endl; // TODO remove
    if (slaves_disc_ == slaves_tot_) {
        go_ = true;
        app_->offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
        std::cout << "############### GO!" << std::endl; // TODO remove
    }
    else {
        go_ = false;
        app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
        std::cout << "############### NOT GO!" << std::endl; // TODO remove
    }

}

/*
 * This is a thread
 */
void CarCTRLClient::run_st() {
    std::cout << "############### Hello, I'm a steering thread!" << std::endl; //TODO remove
    {
        std::unique_lock<std::mutex> init_lk(mu_init_);
        while (!is_init_)
            cond_init_.wait(init_lk);
    }

    std::cout << "############### Steering thread unleashed!" << std::endl; //TODO remove


    while(true) { // TODO some sort of exit condition for program termination
        while(!go_);

        std::unique_lock<std::mutex> req_lk(mu_req_);
        while (app_busy_ && go_)
            cond_req_.wait(req_lk);
        app_busy_ = true;
        std::cout << "############### Steering thread acquired request mutex!" << std::endl;

        if(is_ava_st_) { // TODO put into function which takes ID's as arguments?
            request_->set_service(DIST_STEER_SERVICE_ID);
            request_->set_instance(DIST_STEER_INSTANCE_ID);
            request_->set_method(STEER_METHOD_ID);

            // TODO Replace this block with geting arduio values -------------------------------------
            std::vector<vsomeip::byte_t> data;
            data.push_back(7);
            payload_->set_data(data);
            request_->set_payload(payload_);
            //------------------------------------------------------------------

            app_->send(request_, true);
            std::cout << "############### Steering Requested" << std::endl; // TODO remove
        }

        app_busy_ = false;
        req_lk.unlock();
        cond_req_.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(req_st_sleep_));
    }
}

/*
 * This is a thread
 */
void CarCTRLClient::run_mo() {
    std::cout << "############### Hello, I'm a motor thread!" << std::endl; //TODO remove
    {
        std::unique_lock<std::mutex> init_lk(mu_init_);
        while (!is_init_)
            cond_init_.wait(init_lk);
    }

    std::cout << "############### Motor thread unleashed!" << std::endl; //TODO remove

    while(true) { // TODO some sort of exit condition for program termination
        while(!go_);        

        std::unique_lock<std::mutex> req_lk(mu_req_);
        while (app_busy_)
            cond_req_.wait(req_lk);
        app_busy_ = true;
        std::cout << "############### Motor thread acquired request mutex!" << std::endl; //TODO remove

        if(is_ava_mo_) {
            request_->set_service(MOTOR_SPEED_SERVICE_ID);
            request_->set_instance(MOTOR_SPEED_INSTANCE_ID);
            request_->set_method(MOTOR_METHOD_ID);

            // TODO Replace this block with geting arduio values -------------------------------------
            std::vector<vsomeip::byte_t> data;
            data.push_back(7);
            payload_->set_data(data);
            request_->set_payload(payload_);
            //------------------------------------------------------------------


            app_->send(request_, true);
            std::cout << "############### Motor Requested" << std::endl; // TODO remove
        }

        app_busy_ = false;
        req_lk.unlock();
        cond_req_.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(req_mo_sleep_));
    }
}

















