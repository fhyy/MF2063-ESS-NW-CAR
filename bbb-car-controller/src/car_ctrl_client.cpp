#include "car_ctrl_client.hpp"

#define DEBUG 0
#define WAKEUP_DEBUG 0

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
CarCTRLClient::CarCTRLClient(uint32_t mo_sleep, uint32_t st_sleep, uint32_t setmin_sleep, bool skip_go) :
    run_(false),
    go_(false),
    is_ava_di_(false),
    is_ava_st_(false),
    is_ava_mo_(false),
    is_ava_sp_(false),
    is_ava_cam_(false),
    app_busy_(false),
    shm_mo(CSharedMemory("/shm_mo")),
    shm_st(CSharedMemory("/shm_st")),
    shm_setmin(CSharedMemory("/shm_setmin")),
    shm_shutdown(CSharedMemory("/shm_shutdown")),
    shm_sp(CSharedMemory("/shm_sp")),
    shm_di(CSharedMemory("/shm_di")),
    shm_go(CSharedMemory("/shm_go")),
    shm_cam(CSharedMemory("/shm_cam")),
    req_mo_sleep_(mo_sleep),
    req_st_sleep_(st_sleep),
    req_setmin_sleep_(setmin_sleep),
    skip_go_(skip_go),
    req_shutdown_sleep_(2000) // TODO make settable through CTR?
{
    #if (DEBUG)
        std::cout << "## DEBUG ## car_ctrl_client initializing consumer memory ## DEBUG ##"
                  << std::endl;
    #endif

    int *p;

    // Initialize consumer memory (where input is read from).
    shm_mo.Create(BUFFER_SIZE, O_RDWR);
    shm_mo.Attach(PROT_WRITE);
    p = (int*) shm_mo.GetData();
    buf_mo = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shm_st.Create(BUFFER_SIZE, O_RDWR);
    shm_st.Attach(PROT_WRITE);
    p = (int*) shm_st.GetData();
    buf_st = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shm_setmin.Create(BUFFER_SIZE, O_RDWR);
    shm_setmin.Attach(PROT_WRITE);
    p = (int*) shm_setmin.GetData();
    buf_setmin = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    shm_shutdown.Create(BUFFER_SIZE, O_RDWR);
    shm_shutdown.Attach(PROT_WRITE);
    p = (int*) shm_shutdown.GetData();
    buf_shutdown = Buffer(BUFFER_SIZE, p, B_CONSUMER);

    // Sleep so the program on the other end of the shared memory
    // gets a chance to initialize its consumers.
    sleep(5);

    #if (DEBUG)
        std::cout << "## DEBUG ## car_ctrl_client initializing producer memory ## DEBUG ##"
                  << std::endl;
    #endif

    // Initialize producer memory (where output is written). Hopefully consumers of this memory
    // have been initialized properly while we where sleeping a couple of lines above.
    shm_sp.Create(BUFFER_SIZE, O_RDWR);
    shm_sp.Attach(PROT_WRITE);
    p = (int*) shm_sp.GetData();
    buf_sp = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shm_di.Create(BUFFER_SIZE, O_RDWR);
    shm_di.Attach(PROT_WRITE);
    p = (int*) shm_di.GetData();
    buf_di = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shm_go.Create(BUFFER_SIZE, O_RDWR);
    shm_go.Attach(PROT_WRITE);
    p = (int*) shm_go.GetData();
    buf_go = Buffer(BUFFER_SIZE, p, B_PRODUCER);

    shm_cam.Create(BUFFER_SIZE, O_RDWR);
    shm_cam.Attach(PROT_WRITE);
    p = (int*) shm_cam.GetData();
    buf_cam = Buffer(BUFFER_SIZE, p, B_PRODUCER);
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- init ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool CarCTRLClient::init() {
    // Lock down all threads while initializing.
    std::lock_guard<std::mutex> run_lock(mu_run_);

    // Initialize all threads
    req_mo_thread_ = std::thread(std::bind(&CarCTRLClient::send_motor_req, this));
    req_st_thread_ = std::thread(std::bind(&CarCTRLClient::send_steer_req, this));
    req_setmin_thread_ = std::thread(std::bind(&CarCTRLClient::send_setmin_req, this));
    req_shutdown_thread_ = std::thread(std::bind(&CarCTRLClient::send_shutdown_req, this));

    // Create and initialize applicaiton, payload and request that will hold message data.
    app_ = vsomeip::runtime::get()->create_application("car_ctrl_client");
    if (!app_->init()) {

        // Initialization was unsuccesful.
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();
    request_ = vsomeip::runtime::get()->create_request(false); // false=>UDP

    // Register function that handles behaviour when using app_->start and app_->stop.
    app_->register_state_handler(
        std::bind(&CarCTRLClient::on_state, this,
        std::placeholders::_1));

    // Register function for handling behaviour when availability of services on the network change
    app_->register_availability_handler(vsomeip::ANY_SERVICE,
                                        vsomeip::ANY_INSTANCE,
                                        std::bind(&CarCTRLClient::on_availability,
                                                  this,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2,
                                                  std::placeholders::_3));

    // Register function for handling events published by the distance sensors system
    app_->register_message_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        DIST_EVENT_ID,
        std::bind(&CarCTRLClient::on_dist_eve, this,
                  std::placeholders::_1));

    // Register function for handling events published by the speedometer system
    app_->register_message_handler(
        SPEED_SERVICE_ID,
        SPEED_INSTANCE_ID,
        SPEED_EVENT_ID,
        std::bind(&CarCTRLClient::on_speed_eve, this,
                  std::placeholders::_1));

    // Register function for handling events published by the camera system
    app_->register_message_handler(
        CAM_SERVICE_ID,
        CAM_INSTANCE_ID,
        CAM_EVENT_ID,
        std::bind(&CarCTRLClient::on_cam_eve, this,
                  std::placeholders::_1));

    // Register function for handling emergency break events published by the motor system
    app_->register_message_handler(
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        EMERGENCY_BREAK_EVENT_ID,
        std::bind(&CarCTRLClient::on_embreak_eve, this,
                  std::placeholders::_1));

    // Set all threads free
    run_ = true;
    cond_run_.notify_all();

    // Initialization was succesful
    return true;
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- start ---------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::start() {
    app_->start();

}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- stop ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::stop() {
    req_mo_thread_.join();
    req_st_thread_.join();
    req_setmin_thread_.join();
    req_shutdown_thread_.join();
    go_ = false;
    skip_go_ = false;
    app_->stop();
}

/*
 *#################################################################################################
 *################################# HERE BE PRIVATE MEMBER FUNCTIONS ##############################
 *#################################################################################################
 */

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- update_go_status ----------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::update_go_status() {
    // If the threads are not freed, we don't do anything here. Thus, go_ will always be false.
    if (!run_)
        return;

    // Update go_ depending on which services are avaiable. If go_ is true, bring up the go service
    // to signal to the other nodes that the entire system is online.
    else if (is_ava_di_ && is_ava_st_ &&
             is_ava_mo_ && is_ava_sp_ &&
             is_ava_cam_) {
        go_ = true;
        app_->offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    else {
        go_ = false;
        app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }

    // One hot encoding of which systems are online.
    // LSB represents online status of motor (0 -> NOT online, 1 -> online)
    // second LSB represents online status of steering and so on.
    int service_status = (is_ava_mo_ << 0) |
                         (is_ava_st_ << 1) |
                         (is_ava_sp_ << 2) |
                         (is_ava_di_ << 3) |
                         (is_ava_cam_<< 4);

    // Write the int which holds the encoding to the shared memory.
    shm_go.Lock();
    buf_go.write(service_status);
    shm_go.UnLock();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- send_motor_req ------------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::send_motor_req() {
    // Synchronization lock upon initialization.
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    #if (DEBUG)
        std::cout << "## DEBUG ## send_motor_req thread entering thread loop ## DEBUG ##"
                  << std::endl;
    #endif

    // Thread loop.
    while(run_) {

        // Pause here if !go_.
        while(!(go_ || skip_go_));

        // Store values from shared memory in sensor_data.
        std::vector<int> req_data;

        // Read from the shared memory.
        shm_mo.Lock();
        int unreadValues = buf_mo.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back(buf_mo.read());
        shm_mo.UnLock();

        // Prepare and send transmission if data was read from shared memory.
        if (req_data.size() > 0) {

            // Use only newest request value for transmission.
            unsigned int req_data_latest = req_data.back();

            // Turn int into std::vector of four vsomeip::byte_t.
            std::vector<vsomeip::byte_t> req_data_formatted;
            char byte;
            for (int j=0; j<4; j++) {

                // First element of of vector is lowest 8 bits and so on.
                byte = (req_data_latest >> j*8) & 0xFF;
                req_data_formatted.push_back(byte);
            }

            // Lock down the vsomeip app before sending.
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            // Send the request.
            send_req(req_data_formatted, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, MOTOR_METHOD_ID);

            // Unlock the vsomeip app.
            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();

            #if (DEBUG)
    	        std::cout << "## DEBUG ## Motor request sent: ("
                          << (int) req_data_formatted[0] 
                          << ", " << (int) req_data_formatted[1]
                          << ", " << (int) req_data_formatted[2]
                          << ", " << (int) req_data_formatted[3]
                          << ") ## DEBUG ##" << std::endl;
            #endif
        }

        // Sleep before repeating the thread loop.
        std::this_thread::sleep_for(std::chrono::milliseconds(req_mo_sleep_));
        #if (WAKEUP_DEBUG)
	        std::cout << "## DEBUG ## send_motor_req woke up from sleeping! ## DEBUG ##" << std::endl; 
        #endif
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- send_steer_req ------------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::send_steer_req() {
    // Synchronization lock upon initialization.
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    #if (DEBUG)
        std::cout << "## DEBUG ## send_steer_req thread entering thread loop ## DEBUG ##"
                  << std::endl;
    #endif

    // Thread loop.
    while(run_) {

        // Pause here if !go_.
        while(!(go_ || skip_go_));

        // Store values from shared memory in sensor_data.
        std::vector<int> req_data;

        // Read from the shared memory.
        shm_st.Lock();
        int unreadValues = buf_st.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back(buf_st.read());
        shm_st.UnLock();

        // Prepare and send transmission if data was read from shared memory
        if (req_data.size() > 0) {

            // Use only newest request value for transmission.
            int req_data_latest = req_data.back();

            // Turn int into std::vector of four vsomeip::byte_t.
            std::vector<vsomeip::byte_t> req_data_formatted;
            char byte;
            for (int j=0; j<4; j++) {

                // First element of of vector is lowest 8 bits and so on.
                byte = (req_data_latest >> j*8) & 0xFF;
                req_data_formatted.push_back(byte);
            }

            // Lock down the vsomeip app before sending.
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            // Send the request.
            send_req(req_data_formatted, STEER_SERVICE_ID, STEER_INSTANCE_ID, STEER_METHOD_ID);

            // Unlock the vsomeip app.
            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();

            #if (DEBUG)
    	        std::cout << "## DEBUG ## Steer request sent: ("
                          << (int) req_data_formatted[0] 
                          << ", " << (int) req_data_formatted[1]
                          << ", " << (int) req_data_formatted[2]
                          << ", " << (int) req_data_formatted[3]
                          << ") ## DEBUG ##" << std::endl;
            #endif
        }

        // Sleep before repeating the thread loop.
        std::this_thread::sleep_for(std::chrono::milliseconds(req_st_sleep_));
        #if (WAKEUP_DEBUG)
	        std::cout << "## DEBUG ## send_steer_req woke up from sleeping! ## DEBUG ##"
                      << std::endl; 
        #endif
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- send_setmin_req -----------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::send_setmin_req() {
    // Synchronization lock upon initialization.
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    #if (DEBUG)
        std::cout << "## DEBUG ## send_setmin_req thread entering thread loop ## DEBUG ##"
                  << std::endl;
    #endif

    // Thread loop.
    while(run_) {

      // Pause here if !go_.
        while(!(go_ || skip_go_));

        // Store values from shared memory in sensor_data.
        std::vector<int> req_data;

        // Read from the shared memory.
        shm_setmin.Lock();
        int unreadValues = buf_setmin.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back(buf_setmin.read());
        shm_setmin.UnLock();

        // Prepare and send transmission if data was read from shared memory.
        if (req_data.size() > 0) {

            // Use only newest request values for transmission.
            int req_data_latest = req_data.back();

            // Turn int into std::vector of four vsomeip::byte_t.
            std::vector<vsomeip::byte_t> req_data_formatted;
            char byte;
            for (int j=0; j<4; j++) {

                // First element of of vector is lowest 8 bits and so on.
                byte = (req_data_latest >> j*8) & 0xFF;
                req_data_formatted.push_back(byte);
            }

            // Lock down the vsomeip app before sending.
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;

            // Send the request.
            send_req(req_data_formatted, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, SETMIN_METHOD_ID);

            // Unlock the vsomeip app.
            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();

            #if (DEBUG)
    	        std::cout << "## DEBUG ## Setmin request sent: ("
                          << (int) req_data_formatted[0] 
                          << ", " << (int) req_data_formatted[1]
                          << ", " << (int) req_data_formatted[2]
                          << ", " << (int) req_data_formatted[3]
                          << ") ## DEBUG ##" << std::endl;
            #endif
        }

        // Sleep before repeating the thread loop.
        std::this_thread::sleep_for(std::chrono::milliseconds(req_setmin_sleep_));
        #if (WAKEUP_DEBUG)
	        std::cout << "## DEBUG ## send_setmin_req woke up from sleeping! ## DEBUG ##"
                      << std::endl; 
        #endif
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- send_shutdown_req ---------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::send_shutdown_req() {
    // Synchronization lock upon initialization.
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    #if (DEBUG)
        std::cout << "## DEBUG ## send_shutdown_req thread entering thread loop ## DEBUG ##"
                  << std::endl;
    #endif

    // Thread loop.
    while(run_) {

        // Pause here if !go_.
        while(!(go_ || skip_go_));

        // Store values from shared memory in sensor_data.
        std::vector<int> req_data;

        // Read from the shared memory.
        shm_shutdown.Lock();
        int unreadValues = buf_shutdown.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            req_data.push_back((vsomeip::byte_t) buf_shutdown.read());
        shm_shutdown.UnLock();

        // Prepare and send transmission if data was read from shared memory.
        if (req_data.size() > 0) {

            // Lock down the vsomeip app before sending.
            std::unique_lock<std::mutex> app_lk(mu_app_);
            while (app_busy_)
                cond_app_.wait(app_lk);
            app_busy_ = true;



            /*
             * TODO
             * int req_data_latest = req_data.back();
             * Analyze req_data_latest to see which services the request wants to shut down
             * and the shut down only those services. This needs to be implemented on the
             * ESSPrototype-side and on the respective services. At the time of writing
             * a "shutdown" message ALWAYS shuts down ALL services at the receiver.
             */

            // Dummy data with some random value since the contents does not matter anyway
            // TODO Replace dummy data with ID if the service that will be shut down.
            std::vector<vsomeip::byte_t> dummy_data(7);

            // Send the request.
            // TODO Send the request only to the node which holds the service will be shut down.
            send_req(dummy_data, MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(dummy_data, STEER_SERVICE_ID, STEER_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(dummy_data, SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(dummy_data, DIST_SERVICE_ID, DIST_INSTANCE_ID, SHUTDOWN_METHOD_ID);
            send_req(dummy_data, CAM_SERVICE_ID, CAM_INSTANCE_ID, SHUTDOWN_METHOD_ID);

            // Unlock the vsomeip app.
            app_busy_ = false;
            app_lk.unlock();
            cond_app_.notify_one();

            #if (DEBUG)
    	        std::cout << "## DEBUG ## Shutdown requests sent to all services! ## DEBUG ##"
                          << std::endl;
            #endif
        }

        // Sleep before repeating the thread loop.
        std::this_thread::sleep_for(std::chrono::milliseconds(req_shutdown_sleep_));
        #if (WAKEUP_DEBUG)
	        std::cout << "## DEBUG ## send_shutdown_req woke up from sleeping! ## DEBUG ##"
                      << std::endl;
        #endif
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_dist_eve ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::on_dist_eve(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    // Turn the four-element data packet into an int before writing
    int sensor_data = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];

    #if (DEBUG)
        std::cout << "## DEBUG ## Dist sensor data received by car_ctrl_client: (" << (int) data[0] 
                  << ", " << (int) data[1] << ", " << (int) data[2]
                  << ", " << (int) data[3] << ") ## DEBUG ##" << std::endl;
    #endif

    // Write to shared memory
    shm_di.Lock();
    buf_di.write(sensor_data);
    shm_di.UnLock();

    #if (DEBUG)
        std::cout << "## DEBUG ## Dist sensor data written to shared memory ## DEBUG ##"
                  << std::endl;
    #endif
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_speed_eve --------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::on_speed_eve(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    // Turn the four-element data packet into an int before writing
    int sensor_data = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];

    #if (DEBUG)
        std::cout << "## DEBUG ## Speed sensor data received by car_ctrl_client: ("
                  << (int) data[0]
                  << ", " << (int) data[1] << ", " << (int) data[2]
                  << ", " << (int) data[3] << ") ## DEBUG ##" << std::endl;
    #endif

    // Write to shared memory
    shm_sp.Lock();
    buf_sp.write(sensor_data);
    shm_sp.UnLock();

    #if (DEBUG)
        std::cout << "## DEBUG ## Speed sensor data written to shared memory ## DEBUG ##"
                  << std::endl;
    #endif
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_cam_eve ----------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::on_cam_eve(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    // Turn the four-element data packet into an int before writing
    int sensor_data = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];

    #if (DEBUG)
        std::cout << "## DEBUG ## Camera data received by car_ctrl_client ## DEBUG ##"
                  << std::endl;
    #endif

    // Write to the shared memory
    shm_cam.Lock();
    buf_cam.write(sensor_data);
    shm_cam.UnLock();

    #if (DEBUG)
        std::cout << "## DEBUG ## Camera data written to shared memory ## DEBUG ##"
                  << std::endl;
    #endif
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_embreak_eve ------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::on_embreak_eve(const std::shared_ptr<vsomeip::message> &msg) {
    std::cout << "## WARNING ## Motor has performed an emergency break ## WARNING ##" << std::endl;

    /*
     * TODO
     * Define some appropriate behavior when an emergency break is detected.
     * At the time of writing, the motor service itself sets the speed to 0
     * before publishing the emergency break event.
     */
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- send_req ------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
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

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_state ------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::on_state(vsomeip::state_type_e state) {

    // If we are registered to the vsomeip RTE (by calling app_->start()).
    if(state == vsomeip::state_type_e::ST_REGISTERED) {

        // Subscribe to distance sensor events.
        app_->request_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> dist_group;
        dist_group.insert(DIST_EVENTGROUP_ID);
        app_->request_event(
                DIST_SERVICE_ID,
                DIST_INSTANCE_ID,
                DIST_EVENT_ID,
                dist_group,
                false);
        app_->subscribe(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENTGROUP_ID);

        // Request the steering service.
        app_->request_service(STEER_SERVICE_ID, STEER_INSTANCE_ID);

        // Subscribe to speedometer events.
        app_->request_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> speed_group;
        speed_group.insert(SPEED_EVENTGROUP_ID);
        app_->request_event(
                SPEED_SERVICE_ID,
                SPEED_INSTANCE_ID,
                SPEED_EVENT_ID,
                speed_group,
                false);
        app_->subscribe(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENTGROUP_ID);

        // Subscribe to camera sensor events.
        app_->request_service(CAM_SERVICE_ID, CAM_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> cam_group;
        cam_group.insert(CAM_EVENTGROUP_ID);
        app_->request_event(
                CAM_SERVICE_ID,
                CAM_INSTANCE_ID,
                CAM_EVENT_ID,
                cam_group,
                false);
        app_->subscribe(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENTGROUP_ID);

        // Subscribe to emergency break events.
        app_->request_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> embreak_group;
        embreak_group.insert(EMERGENCY_BREAK_EVENTGROUP_ID);
        app_->request_event(
                MOTOR_SERVICE_ID,
                MOTOR_INSTANCE_ID,
                EMERGENCY_BREAK_EVENT_ID,
                embreak_group,
                false);
        app_->subscribe(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, EMERGENCY_BREAK_EVENTGROUP_ID);
    }

    // If we are NOT registered to the vsomeip RTE (by calling app_->stop()).
    // Basically unsubscribe and unregister from everything.
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

        app_->unsubscribe(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENTGROUP_ID);
        app_->release_event(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENT_ID);
        app_->release_service(CAM_SERVICE_ID, CAM_INSTANCE_ID);

        // If the go server was up when app_->stop() was called we withdraw the service
        // and set go_ to false (which pauses all threads)
        if(go_)
            go_ = false;
            app_->stop_offer_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_availability -----------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CarCTRLClient::on_availability(vsomeip::service_t serv, vsomeip::instance_t inst, bool is_ava) {

    // Is distance sensor service available?
    if (DIST_SERVICE_ID == serv && DIST_INSTANCE_ID == inst) {
        if (is_ava_di_ && !is_ava) {
            is_ava_di_ = false;
        }
        else if (!is_ava_di_ && is_ava) {
            is_ava_di_ = true;
        }
    }

    // Is steering actuator service available?
    else if (STEER_SERVICE_ID == serv && STEER_INSTANCE_ID == inst) {
        if (is_ava_st_ && !is_ava) {
            is_ava_st_ = false;
        }
        else if (!is_ava_st_ && is_ava) {
            is_ava_st_ = true;
        }
    }

    // Is motor actuator service available?
    else if (MOTOR_SERVICE_ID == serv && MOTOR_INSTANCE_ID == inst) {
        if (is_ava_mo_ && !is_ava) {
            is_ava_mo_ = false;
        }
        else if (!is_ava_mo_ && is_ava) {
            is_ava_mo_ = true;
        }
    }

    // Is speedometer sensor service available?
    else if (SPEED_SERVICE_ID == serv && SPEED_INSTANCE_ID == inst) {
        if (is_ava_sp_ && !is_ava) {
            is_ava_sp_ = false;
        }
        else if (!is_ava_sp_ && is_ava) {
            is_ava_sp_ = true;
        }
    }

    // Is camera sensor service available?
    else if (CAM_SERVICE_ID == serv && CAM_INSTANCE_ID == inst) {
        if (is_ava_cam_ && !is_ava) {
            is_ava_cam_ = false;
        }
        else if (!is_ava_cam_ && is_ava) {
            is_ava_cam_ = true;
        }
    }

    // Update the go_ boolean and the availability of the go-service.
    // Write the current availability status to the shared memory.
    update_go_status();
}

/*
 *#################################################################################################
 *################################# MAIN ##########################################################
 *#################################################################################################
 */

// Take care of signal handling if vsomeip was built without signal handling.
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    CarCTRLClient *ccc_ptr(nullptr);

    void handle_signal(int signal) {
        std::cout << "Interrupt signal: " << signal << std::endl;
        if (ccc_ptr != nullptr &&
                (signal == SIGINT || signal == SIGTERM))
            ccc_ptr->stop();
    }
#endif

int main(int argc, char** argv) {

    // Default values for cmdline args.
    uint32_t mo_sleep = 1000;
    uint32_t st_sleep = 1500;
    uint32_t setmin_sleep = 1750;
    bool skip_go = false;

    // Flags for passing cmdline args.
    std::string motor_flag("--motor-sleep");
    std::string steer_flag("--steer-sleep");
    std::string setmin_flag("--setmin-sleep");
    std::string skip_go_flag("--skip-go");

    // See if user passed any flags while starting this program.
    for (int i=1; i<argc; i++) {
        if (steer_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> mo_sleep;
        }
        else if (motor_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> st_sleep;
        }
        else if (setmin_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> setmin_sleep;
        }
        else if (skip_go_flag==argv[i]) {
            skip_go = true;
        }
    }

    // Instatiate client
    CarCTRLClient ccc(mo_sleep, st_sleep, setmin_sleep, skip_go);

    // Take care of signal handling if vsomeip was built without signal handling.
    #ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
        ccc_ptr = &ccc;
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);
    #endif

    // Initialize and start service.
    if (ccc.init()) {
        ccc.start();
        return 0;
    }
    else
        return 1;
}
