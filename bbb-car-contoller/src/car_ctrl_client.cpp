#include "car_ctrl_client.hpp"

CarCTRLClient::CarCTRLClient() :
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
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        EMERGENCY_BREAK_EVENT_ID,
        std::bind(&CarCTRLClient::on_embreak_eve, this,
                  std::placeholders::_1));

    is_init_ = true;
    return true;
}

void CarCTRLClient::start() {
    app_->start();
}

void CarCTRLClient::stop() {
    app_->stop();
    go_ = false;
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
    else if (is_ava_di_ && is_ava_st_ && is_ava_mo_ && is_ava_sp_) {
        go_ = true;
        app_->offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    else {
        go_ = false;
        app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    std::cout << "Distance: " << is_ava_di_ << ", Steer: " << is_ava_st_
	      << ", Motor: " << is_ava_mo_ << ", Speed: " << is_ava_sp_ << std::endl;
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
	std::cout << "POPPING SPEED!!!!!!!!!!!!!!!" << std::endl;
    char data;
    std::unique_lock<std::mutex> q_lk(mu_di_q_);
    while (speed_q_.empty())
        cond_sp_q_.wait(q_lk);
    data = speed_q_.front();
    speed_q_.pop();
    q_lk.unlock();
    std::cout << "Speed popped!!!!!!!!!" << std::endl;
    return data;
}

/*char* CarCTRLClient::pop_distance() {

}*/

/*
 *-------------------------------------------------------------------------------------------------
 *                                  HERE BE PRIVATE MEMBER FUNCTIONS
 *-------------------------------------------------------------------------------------------------
 */

void CarCTRLClient::on_dist_eve(const std::shared_ptr<vsomeip::message> &msg) {
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    std::cout << "DIST EVENT!!!!!!!!!! Data is: (" << (int) data[0] << ", "
              << (int) data[1] << ", " << (int) data[2] << ")" << std::endl;
}

void CarCTRLClient::on_speed_eve(const std::shared_ptr<vsomeip::message> &msg) {
    vsomeip::byte_t *data = msg->get_payload()->get_data();
    std::cout << "SPEED EVENT!!!!!!!!!!!!!! Data is: " << (int) data[0] << std::endl;
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
    update_go_status();
}
