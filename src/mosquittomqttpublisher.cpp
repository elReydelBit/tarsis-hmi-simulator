#include "mosquittomqttpublisher.h"
#include <iostream>

// Constructor: prepares everything needed to send MQTT messages.
MosquittoMqttPublisher::MosquittoMqttPublisher(const std::string &clientId){

    // 1) mosquitto_lib_init() -- MUST run before any other mosquitto
    //    function. It sets up internal library state (e.g. sockets
    //    on Windows). Returns MOSQ_ERR_SUCCESS (0) on success.
    mosquitto_lib_init();

    // 2) mosquitto_new() creates the actual client handle.
    //    Arguments: id (must be unique on the broker),
    //               clean_session (true = don't keep old session state),
    //               obj (a pointer you can attach for callbacks -- we
    //               don't use callbacks yet, so nullptr).
    //    Returns nullptr if creation fails (e.g. out of memory).
    mosq = mosquitto_new(clientId.c_str(), true, nullptr);

    if (!mosq) {

        std::cerr << "Failed to create mosquitto client handle" << std::endl;
        return;  // mosq stays nullptr -- destructor will check this

    }

    // 3) mosquitto_connect() opens the actual network connection
    //    to the broker. Kali is the broker here, default MQTT port 1883,
    //    keepalive 60 = ping the broker every 60s if idle, so it knows
    //    we're still alive.
    int result = mosquitto_connect(mosq, "192.168.1.131", 1883, 60);

    if (result != MOSQ_ERR_SUCCESS) {

        std::cerr << "Failed to connect to MQTT broker" << std::endl;

    } else {

        std::cout << "Connected to MQTT broker at 192.168.1.131" << std::endl;

    }

}

// Destructor: RAII in action -- guarantees cleanup, no matter how
// the object's lifetime ends.
MosquittoMqttPublisher::~MosquittoMqttPublisher(){

    if (mosq) {

        mosquitto_destroy(mosq); // frees the client handle and its connection

    }
    mosquitto_lib_cleanup();     // frees library-wide resources

}

// Publishes a status string (e.g. "ONLINE"/"OFFLINE") to a fixed topic.
// Returns true if the broker accepted the message, false otherwise.
bool MosquittoMqttPublisher::publish(const std::string &payload){

    if (!mosq) {

        std::cerr << "Cannot publish: client handle is not valid" << std::endl;
        return false;

    }

    const char *topic = "tarsis/hmi/status";

    int result = mosquitto_publish(
        mosq,            // this client's handle
        nullptr,         // mid: message ID, not tracked here
        topic,           // where the message goes
        payload.size(),  // payload length, in bytes
        payload.c_str(), // raw bytes of the message
        1,               // QoS 1: broker confirms delivery
        true             // retain: broker keeps this value for late subscribers
    );

    if (result != MOSQ_ERR_SUCCESS) {

        std::cerr << "Failed to publish message" << std::endl;
        return false;

    }

    std::cout << "Published \"" << payload << "\" to topic " << topic << std::endl;
    return true;

}