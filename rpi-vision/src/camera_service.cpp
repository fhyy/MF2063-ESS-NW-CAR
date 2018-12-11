#include "camera_service.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <bitset>

#define DEBUG 1

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- constructor ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
CameraService::CameraService(uint32_t cam_sleep, bool skip_go) :
    run_(false),
    go_(false),
    skip_go_(skip_go),
    pub_cam_sleep_(cam_sleep)
{}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- init ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool CameraService::init() {
    // Lock down the run_cam thread before while initializing.
    std::lock_guard<std::mutex> run_lock(mu_run_);

    // Initialize the camera publisher thread.
    pub_cam_thread_ = std::thread(std::bind(&CameraService::run_cam, this));

    // Create and initialize applicaiton and payload that will hold message data.
    app_ = vsomeip::runtime::get()->create_application("camera_service");
    if (!app_->init()) {
        std::cerr << "Couldn't initialize application" << std::endl;
        return false;
    }
    payload_ = vsomeip::runtime::get()->create_payload();

    // Register function that handles behaviour when using app_->start and app_->stop.
    app_->register_state_handler(
        std::bind(&CameraService::on_state,
                  this,
                  std::placeholders::_1));

    // Register function that handles behaviour when the availability of the go-service changes.
    app_->register_availability_handler(
        GO_SERVICE_ID,
        GO_INSTANCE_ID,
        std::bind(&CameraService::on_go_availability,
                  this,
                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // Register function that handles shutdown requests.
    app_->register_message_handler(
        vsomeip::ANY_SERVICE,
        vsomeip::ANY_INSTANCE,
        SHUTDOWN_METHOD_ID,
        std::bind(&CameraService::on_shutdown, this,
                  std::placeholders::_1));

    // Set the camera notifier thread free.
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
void CameraService::start() {
    app_->start();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- stop ----------------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CameraService::stop() {
    app_->stop();
    run_ = false;
    go_ = false;
    pub_cam_thread_.join();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- is_running ----------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
bool CameraService::is_running() {
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
void CameraService::on_state(vsomeip::state_type_e state) {
    if(state == vsomeip::state_type_e::ST_REGISTERED) {
        // Offer event group for publishing camera events (camera image features).
        app_->offer_service(CAM_SERVICE_ID, CAM_INSTANCE_ID);
        std::set<vsomeip::eventgroup_t> cam_group;
        cam_group.insert(CAM_EVENTGROUP_ID);
        app_->offer_event(CAM_SERVICE_ID,
                          CAM_INSTANCE_ID,
                          CAM_EVENT_ID,
                          cam_group,
                          true);

        // Request the go-service in order to be able to see if bbb-car-controller is ready to go.
        app_->request_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
    else if (state == vsomeip::state_type_e::ST_DEREGISTERED) {
        // Stop the camera event group.
        app_->stop_offer_event(CAM_SERVICE_ID, CAM_INSTANCE_ID, CAM_EVENT_ID);
        app_->stop_offer_service(CAM_SERVICE_ID, CAM_INSTANCE_ID);

        // Stop requesting the go-service.
        app_->release_service(GO_SERVICE_ID, GO_INSTANCE_ID);
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_shutdown ---------------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CameraService::on_shutdown(const std::shared_ptr<vsomeip::message>& msg) {
    #if (DEBUG)
        std::cout << "## DEBUG ## camera_service shutting down! ## DEBUG ##" << std::endl;
    #endif
    stop();
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- on_go_availability --------------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CameraService::on_go_availability(vsomeip::service_t serv,
                                          vsomeip::instance_t inst,
                                          bool go) {
    if (GO_SERVICE_ID == serv && GO_INSTANCE_ID == inst) {
        if (go_ && !go) {
            go_ = false;
            #if (DEBUG)
	            std::cout << "## DEBUG ## camera_service waiting for go-service ## DEBUG ##"
                          << std::endl;
            #endif
        }
        else if (!go_ && go) {
            go_ = true;
            #if (DEBUG)
	            std::cout << "## DEBUG ## camera_service detected go-service ## DEBUG ##"
                          << std::endl;
            #endif
        }
    }
}

/*
 *-------------------------------------------------------------------------------------------------
 *--------------------------------- run_cam -------------------------------------------------------
 *--------------------------------- NOTE: This is a thread ----------------------------------------
 *-------------------------------------------------------------------------------------------------
 */
void CameraService::run_cam() {
    // Synchronization lock upon initialization.
    {
        std::unique_lock<std::mutex> run_lk(mu_run_);
        while (!run_)
            cond_run_.wait(run_lk);
    }

    #if (DEBUG)
    	std::cout << "## DEBUG ## run_cam thread entering thread loop ## DEBUG ##" << std::endl;
    #endif

    // Thread loop.
    while(run_) {

        // Pause here if !go_.
        while(!(go_ || skip_go_));

        int camera_data;
        std::vector<int> camera_vector;
	std::stringstream conv;
        while(true) {
            std::string camera_data_raw;
            getline(std::cin, camera_data_raw);
            conv << camera_data_raw;
            conv >> camera_data;
            if (camera_data == 44 || camera_data_raw.empty()){
                break;
            }
            camera_vector.push_back(camera_data);
        }

        int sensor_data_latest;
        if (camera_vector.size() > 0) {
            // Use only newest sensor value for transmission.
            sensor_data_latest = camera_vector.back();
        }
        else {
            // TODO Send this packet only one
            sensor_data_latest = 0;
        }
        // turn int into std::vector of four vsomeip::byte_t.
        std::vector<vsomeip::byte_t> sensor_data_formatted;
        char byte;
        for (int j=0; j<4; j++) {

            // first element of of vector is lowest 8 bits and so on.
            byte = (sensor_data_latest >> j*8);
            sensor_data_formatted.push_back(byte);
        }

        // Priority (0x0000=low, other=high) could be set here in a future implementation
        // sensor_data_formatted[3] = priority;

        // Set data and publish it on the network.
        payload_->set_data(sensor_data_formatted);
        app_->notify(CAM_SERVICE_ID, CAM_INSTANCE_ID,
                     CAM_EVENT_ID, payload_, true, true);
        #if (DEBUG)
    	    std::cout << "## DEBUG ## Camera sensor data sent: ("
                      << (int) sensor_data_formatted[0] << ", "
                      << (int) sensor_data_formatted[1] << ", "
                      << (int) sensor_data_formatted[2] << ", "
                      << (int) sensor_data_formatted[3] << ") ## DEBUG ##" << std::endl;
        #endif

        //sleep before repeating the thread loop.
        std::this_thread::sleep_for(std::chrono::milliseconds(pub_cam_sleep_));
        #if (DEBUG)
            std::cout << "## DEBUG ## run_cam woke up from sleeping! ## DEBUG ##" << std::endl;
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
    CameraService *cs_ptr(nullptr);

    void handle_signal(int signal) {
        std::cout << "Interrupt signal: " << signal << std::endl;
        if (cs_ptr != nullptr &&
                (signal == SIGINT || signal == SIGTERM))
            cs_ptr->stop();
    }
#endif

int main(int argc, char** argv) {
    // Default values for cmdline args.
    uint32_t cam_sleep = 1000;
    bool skip_go = false;


    // Flags for passing cmdline args.
    std::string sleep_flag("--sleep");
    std::string skip_go_flag("--skip-go"); // should only be used for testing and debugging

    // See if user passed any flags while starting this program.
    for (int i=1; i<argc; i++) {
        if (sleep_flag==argv[i] && i+1<argc) {
            i++;
            std::stringstream conv;
            conv << argv[i];
            conv >> cam_sleep;
        }
        else if (skip_go_flag==argv[i]) {
            skip_go = true;
        }
    }

    // Instatiate service.
    CameraService cs(cam_sleep, skip_go);


    // Take care of signal handling if vsomeip was built without signal handling.
    #ifndef VSOMEIP_ENABLE_SIGNAL_HANDLING
        cs_ptr = &cs;
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);
    #endif

    // Initialize and start service.
    if (cs.init()) {
        cs.start();
        return 0;
    }
    else
        return 1;
}
