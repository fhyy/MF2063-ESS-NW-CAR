#include "car_ctrl_client.hpp"

CarCTRLClient::CarCTRLClient(uint32_t st_sleep, uint32_t mo_sleep) :
    is_init_(false),
    is_ava_st_(false),
    is_ava_mo_(false),
    req_st_sleep_(st_sleep),
    req_mo_sleep_(mo_sleep),
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
    request_ = vsomeip::runtime::get()->create_request(false); // false=>UDP
    payload_ = vsomeip::runtime::get()->create_payload();

    app_->register_state_handler(
        std::bind(&CarCTRLClient::on_state, this,
        std::placeholders::_1));

    app_->register_availability_handler(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID,
        std::bind(&CarCTRLClient::on_availability,
        this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    std::cout << "############### Successfully initialized" << std::endl; // TODO remove

    is_init_ = true;
    cond_init_.notify_all();
    return true;
}

void CarCTRLClient::start() {
    std::cout << "############### Attempting start" << std::endl; // TODO remove
    app_->start();
    std::cout << "############### Successfully started" << std::endl; // TODO remove
}

void CarCTRLClient::on_state(vsomeip::state_type_e state) {
    std::cout << "############### Received state" << std::endl; // TODO remove
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        std::cout << "############### Received state registered" << std::endl; // TODO remove
        app_->request_service(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID);
        // TODO similar request for motor server
    } // TODO handle deregistration
}

void CarCTRLClient::on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool is_ava) {
        if (DIST_STEER_SERVICE_ID == serv && DIST_STEER_INSTANCE_ID == inst) {
            if (is_ava_st_ && !is_ava) {
                is_ava_st_ = false;
                std::cout << "############### STEERING AVAILABLE" << std::endl; // TODO remove
            } else if (!is_ava_st_ && is_ava) {
                is_ava_st_ = true;
                std::cout << "############### STEERING NOT AVAILABLE" << std::endl; // TODO remove
            }
        }
        else {
            std::cout << "############### on_availability triggd, no match" << std::endl; // TODO remove
        }
        // TODO similar block for is_ava_mo_
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

    while(true) { // TODO some sort of exit condition for program termination
        std::unique_lock<std::mutex> req_lk(mu_req_);
        while (app_busy_)
            cond_req_.wait(req_lk);
        app_busy_ = true;
        std::cout << "############### Steering thread acquired request mutex!" << std::endl;

        if(is_ava_st_) { // TODO put into function which takes ID's as arguments?
            request_->set_service(DIST_STEER_SERVICE_ID);
            request_->set_instance(DIST_STEER_INSTANCE_ID);
            request_->set_method(STEER_METHOD_ID);
            // TODO Get arduio values here -------------------------------------
            std::vector<vsomeip::byte_t> data;
            data.push_back(7);
            payload_->set_data(data);
            request_->set_payload(payload_);
            app_->send(request_, true);
            //------------------------------------------------------------------
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

    while(true) { // TODO some sort of exit condition for program termination
        std::unique_lock<std::mutex> req_lk(mu_req_);
        while (app_busy_)
            cond_req_.wait(req_lk);
        app_busy_ = true;
        std::cout << "############### Motor thread acquired request mutex!" << std::endl; //TODO remove

        if(is_ava_mo_) {
            request_->set_service(MOTOR_SPEED_SERVICE_ID);
            request_->set_instance(MOTOR_SPEED_INSTANCE_ID);
            request_->set_method(MOTOR_METHOD_ID);
            // TODO Get arduio values here -------------------------------------
            std::vector<vsomeip::byte_t> data;
            data.push_back(7);
            payload_->set_data(data);
            request_->set_payload(payload_);
            app_->send(request_, true);
            std::cout << "############### Motor Requested" << std::endl; // TODO remove
            //------------------------------------------------------------------
        }

        app_busy_ = false;
        req_lk.unlock();
        cond_req_.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(req_mo_sleep_));
    }
}

















