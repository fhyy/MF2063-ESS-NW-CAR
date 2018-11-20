#include "car_ctrl_client.hpp"

CarCTRLClient::CarCTRLClient(uint32_t n) :
    services_tot_(n),
    services_disc_(0),
    is_init_(false),
    go_(false),
    is_ava_di_(false),
    is_ava_st_(false),
    is_ava_mo_(false),
    is_ava_sp_(false),
    app_busy_(false)
    {}

bool CarCTRLClient::init() {
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

    /* TODO app_->register_message_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        DIST_EVENT_ID,
        std::bind(&CarCTRLClient::on_dist_eve, this,
                  std::placeholders::_1));*/

    app_->register_message_handler(
        SPEED_SERVICE_ID,
        SPEED_INSTANCE_ID,
        SPEED_EVENT_ID,
        std::bind(&CarCTRLClient::on_speed_eve, this,
                  std::placeholders::_1));

    is_init_ = true;
    return true;
}

void CarCTRLClient::start() {
    app_->start();
}

bool CarCTRLClient::check_availability(vsomeip::service_t serv, vsomeip::instance_t inst) {
    if (!is_init_)
        return false;
    else if (DIST_SERVICE_ID == serv && DIST_INSTANCE_ID == inst)
        return is_ava_di_;
    else if (STEER_SERVICE_ID == serv && STEER_INSTANCE_ID == inst)
        return is_ava_st_;
    else if (MOTOR_SERVICE_ID == serv && MOTOR_INSTANCE_ID == inst)
        return is_ava_mo_;
    else if (SPEED_SERVICE_ID == serv && SPEED_INSTANCE_ID == inst)
        return is_ava_sp_;
}

bool CarCTRLClient::update_go_status() {
    if (!is_init_)
        return false;        
    else if (services_disc_ == services_tot_) {
        go_ = true;
        app_->offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    else {
        go_ = false;
        app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    return go_;
}

void CarCTRLClient::send_motor_req(char s, bool prio) {
    std::vector<vsomeip::byte_t> data;
    if (prio)
        data.push_back(0xF0);
    else
        data.push_back(0x00);
    data.push_back(s);

    std::unique_lock<std::mutex> req_lk(mu_app_);
    while (app_busy_)
        cond_app_.wait(req_lk);
    app_busy_ = true;

    send_req(data, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, MOTOR_METHOD_ID);

    app_busy_ = false;
    req_lk.unlock();
    cond_app_.notify_one();
}

void CarCTRLClient::send_motor_req(char s, char a, bool prio) {
    std::vector<vsomeip::byte_t> data;
    if (prio)
        data.push_back(0xFF);
    else
        data.push_back(0x00);
    data.push_back(s);
    data.push_back(a);

    std::unique_lock<std::mutex> req_lk(mu_app_);
    while (app_busy_)
        cond_app_.wait(req_lk);
    app_busy_ = true;

    send_req(data, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, MOTOR_METHOD_ID);

    app_busy_ = false;
    req_lk.unlock();
    cond_app_.notify_one();
}

void CarCTRLClient::send_steer_req(char d, bool prio) {
    std::vector<vsomeip::byte_t> data;
    if (prio)
        data.push_back(0xF0);
    else
        data.push_back(0x00);
    data.push_back(d);

    std::unique_lock<std::mutex> req_lk(mu_app_);
    while (app_busy_)
        cond_app_.wait(req_lk);
    app_busy_ = true;

    send_req(data, STEER_SERVICE_ID, STEER_INSTANCE_ID, STEER_METHOD_ID);

    app_busy_ = false;
    req_lk.unlock();
    cond_app_.notify_one();
}

char CarCTRLClient::pop_speed() {
    char data;
    std::unique_lock<std::mutex> q_lk(mu_di_q_);
    while (speed_q_.empty())
        cond_sp_q_.wait(q_lk);
    data = speed_q_.front();
    speed_q_.pop();
    q_lk.unlock();
    return data;
}

/*char* CarCTRLClient::pop_distance() {

}*/

/*
 *-------------------------------------------------------------------------------------------------
 *                                  HERE BE PRIVATE MEMBER FUNCTIONS
 *-------------------------------------------------------------------------------------------------
 */

/*void CarCTRLClient::on_dist_eve(const std::shared_ptr<vsomeip::message> &_msg) {
    //TODO extract sensor data from msg
    char data[3] = {7, 7, 7};
    std::unique_lock<std::mutex> q_lk(mu_di_q_);
    if (dist_q_.size() == DIST_Q_DEPTH) // Pop oldest data if queue is full
        dist_q_.pop();
    dist_q_.push(data);

    q_lk.unlock();
    cond_di_q_.notify_one();
}*/

void CarCTRLClient::on_speed_eve(const std::shared_ptr<vsomeip::message> &_msg) {
    //TODO extract sensor data from msg
    char data = 7;
    std::unique_lock<std::mutex> q_lk(mu_sp_q_);
    if (speed_q_.size() == SPEED_Q_DEPTH) // Pop oldest data if queue is full
        speed_q_.pop();
    speed_q_.pop();

    q_lk.unlock();
    cond_sp_q_.notify_one();
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
        app_->request_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);
        app_->request_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
        app_->request_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);

        std::set<vsomeip::eventgroup_t> dist_group;
        dist_group.insert(DIST_EVENTGROUP_ID);
        app_->request_event(
                DIST_SERVICE_ID,
                DIST_INSTANCE_ID,
                DIST_EVENT_ID,
                dist_group,
                false); // TODO what does this boolean do?
        app_->subscribe(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENTGROUP_ID);

        std::set<vsomeip::eventgroup_t> speed_group;
        dist_group.insert(SPEED_EVENTGROUP_ID);
        app_->request_event(
                SPEED_SERVICE_ID,
                SPEED_INSTANCE_ID,
                SPEED_EVENT_ID,
                speed_group,
                false); // TODO what does this boolean do?
        app_->subscribe(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENTGROUP_ID);

        update_go_status();
    }
    else if(state == vsomeip::state_type_e::ST_DEREGISTERED) {
        app_->release_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);
        app_->release_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);
        app_->release_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
        app_->release_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);

        if(go_)
            go_ = false;
            app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
}

void CarCTRLClient::on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool is_ava) {
    if (DIST_SERVICE_ID == serv && DIST_INSTANCE_ID == inst) {
        if (is_ava_di_ && !is_ava) {
            is_ava_di_ = false;
            services_disc_--;
        }
        else if (!is_ava_di_ && is_ava) {

            is_ava_di_ = true;
            services_disc_++;
        }
    }
    else if (STEER_SERVICE_ID == serv && STEER_INSTANCE_ID == inst) {
        if (is_ava_st_ && !is_ava) {
            is_ava_st_ = false;
            services_disc_--;
        }
        else if (!is_ava_st_ && is_ava) {
            is_ava_st_ = true;
            services_disc_++;
        }
    }
    else if (MOTOR_SERVICE_ID == serv && MOTOR_INSTANCE_ID == inst) {
        if (is_ava_mo_ && !is_ava) {
            is_ava_mo_ = false;
            services_disc_--;
        }
        else if (!is_ava_mo_ && is_ava) {
            is_ava_mo_ = true;
            services_disc_++;
        }
    }
    else if (SPEED_SERVICE_ID == serv && SPEED_INSTANCE_ID == inst) {
        if (is_ava_sp_ && !is_ava) {
            is_ava_sp_ = false;
            services_disc_--;
        }
        else if (!is_ava_sp_ && is_ava) {
            is_ava_sp_ = true;
            services_disc_++;
        }
    }
}
