#include "dist_steer_service.hpp"

DistSteerService::DistSteerService(uint32_t di_sleep) :
    is_init_(false),
    go_(false),
    pub_di_sleep_(di_sleep)
    {}

bool DistSteerService::init() {
    std::lock_guard<std::mutex> init_lock(mu_init_);

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
        std::bind(&DistSteerService::on_go,
        this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    app_->register_message_handler(
        DIST_STEER_SERVICE_ID,
        DIST_STEER_INSTANCE_ID,
        STEER_METHOD_ID,
        std::bind(&DistSteerService::on_steer_req, this,
                  std::placeholders::_1));

    is_init_ = true;
    cond_init_.notify_all();

    return true;
}

void DistSteerService::start() {
    app_->start();
    std::cout << "################## Succesfully started!" << std::endl; // TODO remove
}

void DistSteerService::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        app_->offer_service(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID);

        std::set<vsomeip::eventgroup_t> dist_group;
        dist_group.insert(DIST_EVENTGROUP_ID);
        app_->offer_event(DIST_STEER_SERVICE_ID,
                          DIST_STEER_INSTANCE_ID,
                          DIST_EVENT_ID,
                          dist_group,
                          true); //TODO what does this boolean do?

        app_->request_service(GO_SERVICE_ID, GO_INSTANCE_ID);

        std::cout << "################ Succesfully registered at runtime!" << std::endl; // TODO remove
    }
}

void DistSteerService::on_steer_req(const std::shared_ptr<vsomeip::message> &_msg) {
    std::cout << "#################### Got Message!" << std::endl;
}

void DistSteerService::on_go(vsomeip::service_t serv, vsomeip::instance_t inst, bool go) {
    std::cout << "############### Go status:" << go_ <<std::endl; // TODO remove
    std::cout << "############### Init:" << is_init_ <<std::endl; // TODO remove
    if (GO_SERVICE_ID == serv && GO_INSTANCE_ID == inst) {
        if (go_ && !go) {
            go_ = false;
            std::cout << "############### NOT GO!" << std::endl; // TODO remove
        }
        else if (!go_ && go) {
            go_ = true;
            std::cout << "############### GO!" << std::endl; // TODO remove
        }
    }
    else {
        std::cout << "############### on_go triggd, no match" << std::endl; // TODO remove
    }

    std::cout << "############### Go status:" << go_ <<std::endl; // TODO remove
    std::cout << "############### Init:" << is_init_ <<std::endl; // TODO remove
}

void DistSteerService::run_di() {
    std::cout << "############### Hello, I'm a dist thread!" << std::endl; //TODO remove
    {
        std::unique_lock<std::mutex> init_lk(mu_init_);
        while (!is_init_) // TODO pause via go_?
            cond_init_.wait(init_lk);
    }

    std::cout << "############### Dist thread unleashed!" << std::endl; //TODO remove

    while(true) { // TODO pause via go_?
        while(!go_);

        // TODO Replace this block with geting arduio values -------------------------------------
        std::vector<vsomeip::byte_t> data;
        data.push_back(7);
        data.push_back(7);
        data.push_back(7);
        data.push_back(7);
        data.push_back(7);
        data.push_back(7);
        payload_->set_data(data);
        //------------------------------------------------------------------
        
        app_->notify(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID,
                     DIST_EVENT_ID, payload_, true, true);
        std::cout << "############### Dist thread just notified!" << std::endl; //TODO remove
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_di_sleep_));
        std::cout << "############### Dist thread just awoke!" << std::endl; //TODO remove
    }
}








