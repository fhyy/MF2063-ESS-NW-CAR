#include "motor_speed_service.hpp"

#define DEBUG 1 //Set to 1 if you want cout messages to aid debugging, 0 if you don't.

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
MotorSpeedService::MotorSpeedService(uint32_t sp_sleep, uint8_t min_dist, bool skip_go) :
    run_(false),
    go_(false),
    use_dist_(false),
    shm_sp(CSharedMemory("/shm_sp")),
    shm_mo(CSharedMemory("/shm_mo")),
    pub_sp_sleep_(sp_sleep),
    min_dist_(min_dist),
    skip_go_(skip_go)
{
    #if (DEBUG)
        std::cout << "## DEBUG ## motor_speed_service initializing consumer memory ## DEBUG ##"
                  << std::endl;
    #endif


    // Initialize consumer memory (where input is read from).
    try {
        shm_sp.Create(BUFFER_SIZE, O_RDWR);
        shm_sp.Attach(PROT_WRITE);
    }
    catch (CSharedMemoryException& e) {
        std::cout << e.what() << std::endl;
    }
    int *p_sp = (int*) shm_sp.GetData();
    buf_sp = Buffer(BUFFER_SIZE, p_sp, B_CONSUMER);

    // Sleep so the program on the other end of the shared memory
    // gets a chance to initialize its consumers.
    sleep(2);

    #if (DEBUG)
        std::cout << "## DEBUG ## motor_speed_service initializing producer memory ## DEBUG ##"
                  << std::endl;
    #endif

    // Initialize producer memory (where output is written). Hopefully consumers of this memory
    // have been initialized properly while we where sleeping a couple of lines above.
    try {
        shm_mo.Create(BUFFER_SIZE, O_RDWR);
        shm_mo.Attach(PROT_WRITE);
    }
    catch (CSharedMemoryException& e) {
        std::cout << e.what() << std::endl;
    }
    int *p_mo = (int*) shm_mo.GetData();
    buf_mo = Buffer(BUFFER_SIZE, p_mo, B_PRODUCER);

}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- init ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool MotorSpeedService::init() {
    // Lock down the run_sp thread before while initializing.
    std::lock_guard<std::mutex> run_lock(mu_run_);

    // Initialize the speed publisher thread.
    pub_sp_thread_ = std::thread(std::bind(&MotorSpeedService::run_sp, this));

    // Create and initialize applicaiton and payload that will hold message data.
    app_ = vsomeip::runtime::get()->create_application("motor_speed_service");
    if (!app_->init()) {

        // Initialization was unsuccesful.
        std::cerr << "Couldn't initialize application" << std::endl;
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();

    // Register function that handles behaviour when using app_->start and app_->stop.
    app_->register_state_handler(
        std::bind(&MotorSpeedService::on_state,
                  this,
                  std::placeholders::_1));

    // Register function that handles behaviour when the availability of the go-service changes.
    app_->register_availability_handler(
        GO_SERVICE_ID,
        GO_INSTANCE_ID,
        std::bind(&MotorSpeedService::on_go_availability,
                  this,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // Register fnc that handles behaviour when the availability of the distance service changes.
    app_->register_availability_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        std::bind(&MotorSpeedService::on_dist_availability,
                  this,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // Register function that handles behaviour when distance events are detected.
    app_->register_message_handler(
        DIST_SERVICE_ID,
        DIST_INSTANCE_ID,
        DIST_EVENT_ID,
        std::bind(&MotorSpeedService::on_dist_eve,
                  this,
                  std::placeholders::_1));

    // Register function that handles motor requests.
    app_->register_message_handler(
        MOTOR_SERVICE_ID,
        MOTOR_INSTANCE_ID,
        MOTOR_METHOD_ID,
        std::bind(&MotorSpeedService::on_motor_req,
                  this,
                  std::placeholders::_1));

    // Register function that handles setmin requests.
    app_->register_message_handler(
        MOTOR_SERVICE_ID,
        MOTOR_INSTANCE_ID,
        SETMIN_METHOD_ID,
        std::bind(&MotorSpeedService::on_setmin_req,
                  this,
                  std::placeholders::_1));

    // Register function that handles shutdown requests.
    app_->register_message_handler(
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        SHUTDOWN_METHOD_ID,
        std::bind(&MotorSpeedService::on_shutdown, this,
                  std::placeholders::_1));

    // Set the distance notifier thread free.
    run_ = true;
    cond_run_.notify_all();

    // Initialization was succesful.
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
    skip_go_ = false;
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
        // Offer service for handling motor and setmin requests and
        // for sending emergency break events.
        app_->offer_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> embreak_group;
        embreak_group.insert(EMERGENCY_BREAK_EVENTGROUP_ID);
        app_->offer_event(MOTOR_SERVICE_ID,
                          MOTOR_INSTANCE_ID,
                          EMERGENCY_BREAK_EVENT_ID,
                          embreak_group,
                          true); //TODO what does this boolean do?

        // Offer event group for publishing speed events (speed sensor data).
        app_->offer_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> speed_group;
        speed_group.insert(SPEED_EVENTGROUP_ID);
        app_->offer_event(SPEED_SERVICE_ID,
                          SPEED_INSTANCE_ID,
                          SPEED_EVENT_ID,
                          speed_group,
                          true); //TODO what does this boolean do?

        // Request the go-service in order to be able to see if bbb-car-controller is ready to go.
        app_->request_service(GO_SERVICE_ID, GO_INSTANCE_ID);

        // Request the distance service and join the eventgroup to be able to see distance events.
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
    }
    else if (state == vsomeip::state_type_e::ST_DEREGISTERED) {
        // Stop the speed event group.
        app_->stop_offer_event(SPEED_SERVICE_ID, SPEED_INSTANCE_ID, SPEED_EVENT_ID);
        app_->stop_offer_service(SPEED_SERVICE_ID, SPEED_INSTANCE_ID);

        // Stop offering the motor service and the emergency break event group.
        app_->stop_offer_event(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID, EMERGENCY_BREAK_EVENT_ID);
        app_->stop_offer_service(MOTOR_SERVICE_ID, MOTOR_INSTANCE_ID);

        // Stop requesting the go-service.
        app_->release_service(GO_SERVICE_ID, GO_INSTANCE_ID);

        // Stop subscription to the distance event group.
        app_->unsubscribe(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENTGROUP_ID);
        app_->release_event(DIST_SERVICE_ID, DIST_INSTANCE_ID, DIST_EVENT_ID);
        app_->release_service(DIST_SERVICE_ID, DIST_INSTANCE_ID);

        // Write to shared memory (stop the car because we are not running vsomeip anymore)
        shm_mo.Lock();
        buf_mo.write(0);
        shm_mo.UnLock();

    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_dist_eve ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_dist_eve(const std::shared_ptr<vsomeip::message> &msg) {
    // If distance service has not been detected we don't do anything.
    // (This function will probably not ever be invoked if distance service is down.) 
    if (!use_dist_)
        return;

    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    #if (DEBUG)
        std::cout << "## DEBUG ## Dist sensor data received by motor_speed_service: (" << (int) data[0] 
                  << ", " << (int) data[1] << ", " << (int) data[2]
                  << ", " << (int) data[3] << ") ## DEBUG ##" << std::endl;
    #endif

    // If any sensor value falls below distance threshold create and send an embreak packet
    if (data[0]<min_dist_ || data[1]<min_dist_ || data[2]<min_dist_) {

        // Create variables for storing data
        std::shared_ptr<vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();
        std::vector<vsomeip::byte_t> dummy_data;

        // Load embreak packet with dummy data
        dummy_data.push_back(13);
        payload->set_data(dummy_data);

        // Send out embreak packet
	    app_->notify(MOTOR_SERVICE_ID,
                        MOTOR_INSTANCE_ID,
                        EMERGENCY_BREAK_EVENT_ID,
                        payload,
                        true, true);

        // Set speed to 0
        shm_mo.Lock();
        buf_mo.write(0);
        shm_mo.UnLock();

        #if (DEBUG)
        	std::cout << "## DEBUG ## Embreak event sent! Min distance threshold: " << (int) min_dist_ 
                      << " ## DEBUG ##" << std::endl;
        #endif
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_motor_req --------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_motor_req(const std::shared_ptr<vsomeip::message> &msg) {
    // Store received packet, should always have a length of 4
    vsomeip::byte_t *data = msg->get_payload()->get_data();

    // Turn the four-element data packet into an int before writing
    int req = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]);

    #if (DEBUG)
        std::cout << "## DEBUG ## Got motor request!! Data: (" << (int) data[0]
                  << ", " << (int) data[1] << ", " << (int) data[2] << ", " << (int) data[3]
                  << ") ## DEBUG ##" << std::endl;
    #endif

    // Write to shared memory
    shm_mo.Lock();
    buf_mo.write(req);
    shm_mo.UnLock();

    #if (DEBUG)
        std::cout << "## DEBUG ## Motor request written to shared memory ## DEBUG ##" << std::endl;
    #endif
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_setmin_req -------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_setmin_req(const std::shared_ptr<vsomeip::message> &msg) {
    // first element of a setmin packet will always be what we are looking for
    min_dist_ = msg->get_payload()->get_data()[0];

    #if (DEBUG)
        std::cout << "## DEBUG ## Got setmin request!! New distance threshold: " << (int) min_dist_
                  << "## DEBUG ##" << std::endl;
    #endif
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_shutdown ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void MotorSpeedService::on_shutdown(const std::shared_ptr<vsomeip::message>& msg) {
    #if (DEBUG)
        std::cout << "## DEBUG ## motor_speed_service shutting down! ## DEBUG ##" << std::endl;
    #endif
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

    // Update go_ whenever the availability of the go-server changes.
    if (GO_SERVICE_ID == serv && GO_INSTANCE_ID == inst) {
        if (go_ && !go) {
            go_ = false;

            // Write to shared memory (stop the car because services are unavailable)
            shm_mo.Lock();
            buf_mo.write(0);
            shm_mo.UnLock();

            #if (DEBUG)
	            std::cout << "## DEBUG ## motor_speed_service waiting for go-service ## DEBUG ##"
                          << std::endl;
            #endif
        }
        else if (!go_ && go) {
            go_ = true;
            #if (DEBUG)
	            std::cout << "## DEBUG ## motor_speed_service detected go-service ## DEBUG ##"
                          << std::endl;
            #endif
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

    // Update use_dist_ whenever the availability of the distance service changes.
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
    { // Synchronization lock upon initialization.
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    #if (DEBUG)
        std::cout << "## DEBUG ## run_sp thread entering thread loop ## DEBUG ##" << std::endl;
    #endif

    // Thread loop.
    while(run_) {

        // Pause here if !go_.
        while(!(go_ || skip_go_));

        // Store values from shared memory in sensor_data.
        std::vector<int> sensor_data;

        // Read from the shared memory.
        shm_sp.Lock();
        int unreadValues = buf_sp.getUnreadValues();
        for (int i=0; i<unreadValues; i++)
            sensor_data.push_back(buf_sp.read());
        shm_sp.UnLock();

        // prepare and send transmission if data was read from shared memory.
        if (sensor_data.size() > 0) {

            // Use only newest sensor value for transmission.
            int sensor_data_latest = sensor_data.back();

            // turn int into std::vector of four vsomeip::byte_t.
            std::vector<vsomeip::byte_t> sensor_data_formatted;
            char byte;
            for (int j=0; j<4; j++) {

                // first element of of vector is lowest 8 bits and so on.
                byte = (sensor_data_latest >> j*8);
                sensor_data_formatted.push_back(byte);
            }

            // Priority (0x0000=low, other=high) could be set here in a future implementation.
            // sensor_data_formatted[3] = priority;

            // set data and publish it on the network.
            payload_->set_data(sensor_data_formatted);
            app_->notify(SPEED_SERVICE_ID, SPEED_INSTANCE_ID,
                         SPEED_EVENT_ID, payload_, true, true);

            #if (DEBUG)
    	        std::cout << "## DEBUG ## Speed sensor data sent: ("
                          << (int) sensor_data_formatted[0]
                          << ", " << (int) sensor_data_formatted[1]
                          << ", " << (int) sensor_data_formatted[2]
                          << ", " << (int) sensor_data_formatted[3]
                          << ") ## DEBUG ##" << std::endl;
            #endif
        }

        //sleep before repeating the thread loop.
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_sp_sleep_));
        #if (DEBUG)
	        std::cout << "## DEBUG ## run_sp woke up from sleeping! ## DEBUG ##" << std::endl; 
        #endif
    }
}

/*
 *#################################################################################################
 *################################# MAIN ##########################################################
 *#################################################################################################
 */

// Take care of signal handling if vsomeip was built without signal handling.
#ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
    MotorSpeedService *mss_ptr(nullptr);

    void handle_signal(int signal) {
        std::cout << " ## SIGNAL ## Interrupt signal: " << signal << " ## SIGNAL ##" <<std::endl;
        if (mss_ptr != nullptr &&
                (signal == SIGINT || signal == SIGTERM))
            mss_ptr->stop();
    }
#endif

int main(int argc, char** argv) {
    // Default values for cmdline args.
    uint32_t sp_sleep = 3000;
    uint8_t min_dist = 30;
    bool skip_go = false;

    // Flags for passing cmdline args.
    std::string sleep_flag("--sleep");
    std::string min_dist_flag("--min-dist");
    std::string skip_go_flag("--skip-go");

    // See if user passed any flags while starting this program.
    for (int i=1; i<argc; i++) {
        if (sleep_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> sp_sleep;
        }
        else if (min_dist_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> min_dist;
        }
        else if (skip_go_flag==argv[i]) {
            skip_go = true;
        }
    }

    // Instatiate service.
    MotorSpeedService mss(sp_sleep, min_dist, skip_go);

    // Take care of signal handling if vsomeip was built without signal handling.
    #ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
        mss_ptr = &mss;
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);
    #endif

    // Initialize and start service.
    if (mss.init()) {
        mss.start();
        return 0;
    }
    else
        return 1;
}
