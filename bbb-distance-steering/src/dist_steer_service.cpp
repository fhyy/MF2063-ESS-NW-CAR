#include "dist_steer_service.hpp"

DistSteerService::DistSteerService() :
    is_init_(false)
    {}

bool DistSteerService::init() {
    std::lock_guard<std::mutex> init_lock(mu_init_);

    app_ = vsomeip::runtime::get()->create_application("dist_steer_service");

    if (!app_->init()) {
        std::cerr << "Couldn't initialize application" << std::endl;
        return false;
    }

    app_->register_state_handler(
        std::bind(&DistSteerService::on_state, this,
        std::placeholders::_1));

    app_->register_message_handler(
        DIST_STEER_SERVICE_ID,
        DIST_STEER_INSTANCE_ID,
        STEER_METHOD_ID,
        std::bind(&DistSteerService::on_steer_req, this,
                  std::placeholders::_1));

    return true;
}

void DistSteerService::start() {
    app_->start();
    std::cerr << "################## Succesfully started!" << std::endl; // TODO remove
}

void DistSteerService::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        app_->offer_service(DIST_STEER_SERVICE_ID, DIST_STEER_INSTANCE_ID);
        is_init_ = true;
        std::cerr << "################ Succesfully registered at runtime!" << std::endl; // TODO remove
        //cond_init_.notify_one();
    }
}

void DistSteerService::on_steer_req(const std::shared_ptr<vsomeip::message> &_msg) {
    std::cout << "#################### Got Message!" << std::endl;
}
