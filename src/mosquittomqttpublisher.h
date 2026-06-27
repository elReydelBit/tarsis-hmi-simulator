#pragma once

#include <mosquitto.h> // C API: struct mosquitto, mosquitto_new, mosquitto_connect, etc.
#include <string>

// Wraps a single mosquitto client handle dedicated to publishing.
// "class" (not struct) because there IS an invariant to protect:
// nobody outside should be able to touch the raw "mosq" pointer
// directly -- doing so could skip mosquitto_destroy() and leak
// the underlying resource (socket, internal buffers).
class MosquittoMqttPublisher{

    private:
        //struct mosquitto *mosq; // opaque handle -- this client's connection

         mosquitto *mosq;   // "struct" omitted on purpose: in C++, a forward-declared
                            // struct tag is automatically usable as a type name,
                            // no typedef needed (this is different from plain C).

    
    public:
        MosquittoMqttPublisher (const std::string &clientId);
        bool publish(const std::string &payload);
        ~MosquittoMqttPublisher();
        


};