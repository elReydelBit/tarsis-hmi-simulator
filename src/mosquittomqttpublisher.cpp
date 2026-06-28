#include "mosquittomqttpublisher.h"
#include <iostream>

// Constructor: prepares everything needed to send MQTT messages.
MosquittoMqttPublisher::MosquittoMqttPublisher(const std::string &clientId){

    // 1) mosquitto_lib_init() -- MUST run before any other mosquitto
    //    function. It sets up internal library state (e.g. sockets
    //    on Windows). Returns MOSQ_ERR_SUCCESS (0) on success.
    mosquitto_lib_init();

    
    // Antes pasábamos nullptr aquí porque no usábamos callbacks. Ahora
    // pasamos "this" - es el mecanismo real para que un callback estático
    // (declarado en la Pieza A) pueda recuperar el objeto concreto al que
    // pertenece: mosquitto nos devolverá este mismo puntero como "obj"
    // cada vez que llame a onConnectCallback/onDisconnectCallback.
    mosq = mosquitto_new(clientId.c_str(), true, this);

    // Le decimos a la librería qué función llamar cuando el ESTADO REAL
    // de la conexión cambie - esto sustituye al "asumir éxito" que teníamos
    // antes. Por sí solo, esto todavía NO hace nada: mosquitto solo dispara
    // estos callbacks cuando hay un hilo corriendo su bucle de red (Paso 4,
    // más abajo) - sin eso, nunca llegan a ejecutarse, da igual cómo
    // cambie la conexión.
    mosquitto_connect_callback_set(mosq, onConnectCallback);
    mosquitto_disconnect_callback_set(mosq, onDisconnectCallback);

   

    // mosquitto_connect() ahora solo ENVÍA la petición de conexión - no
    // espera respuesta del broker, así que su resultado ya no nos dice si
    // estamos conectados de verdad (eso es justo lo que queríamos arreglar).
    // "connected" se queda en su valor inicial (false) hasta que
    // onConnectCallback confirme la conexión real, más adelante, en el
    // hilo en segundo plano.
    int result = mosquitto_connect(mosq, "192.168.1.131", 1883, 60);

    if (result != MOSQ_ERR_SUCCESS) {

        std::cerr << "Failed to send MQTT connect request" << std::endl;

    }

    // mosquitto_loop_start() crea un hilo NUEVO, separado del hilo principal
    // de Qt, dedicado solo a escuchar la red de mosquitto: completar la
    // conexión, mantener el keepalive, y disparar los callbacks de arriba
    // en el momento real en que ocurren. Sin esto, mosquitto nunca "hace
    // nada por sí solo" - todo lo que teníamos antes dependía de que el
    // hilo de Qt llamara a una función de mosquitto en algún punto.
    mosquitto_loop_start(mosq);

}

// Exposes the real result of the connect attempt made above - no
// guessing, no assuming success just because the object exists.
bool MosquittoMqttPublisher::isConnected() const {

    return this->connected;

}


// Llamado por mosquitto, desde SU hilo en segundo plano, en el instante
// real en que el broker confirma (o rechaza) la conexión. "obj" es el
// "this" que guardamos en la Pieza B - lo recuperamos con static_cast
// para poder tocar el objeto concreto (no hay otra forma: esta función
// es estática, no tiene "this" propio, por eso necesita que se lo pasen).
void MosquittoMqttPublisher::onConnectCallback(struct mosquitto *mosq, void *obj, int rc)
{
    MosquittoMqttPublisher *self = static_cast<MosquittoMqttPublisher*>(obj);

    // rc == 0 es el único valor que significa "el broker aceptó la
    // conexión". Cualquier otro número es un motivo de rechazo distinto
    // (credenciales, protocolo, etc. - no los distinguimos aquí, de
    // momento solo nos importa éxito/fallo).
    self->connected = (rc == 0);

    if (rc == 0) {
        std::cout << "MQTT connected (confirmado por callback)" << std::endl;
    } else {
        std::cerr << "MQTT connect callback: fallo, rc=" << rc << std::endl;
    }
}

// Mismo mecanismo que el de arriba, pero para cuando la conexión se
// PIERDE (Kali se apaga, se corta la red, etc.) - sin esto, "connected"
// se quedaría en "true" para siempre aunque el broker ya no esté ahí.
void MosquittoMqttPublisher::onDisconnectCallback(struct mosquitto *mosq, void *obj, int rc)
{
    MosquittoMqttPublisher *self = static_cast<MosquittoMqttPublisher*>(obj);
    self->connected = false;

    std::cout << "MQTT disconnected (confirmado por callback), rc=" << rc << std::endl;
}


MosquittoMqttPublisher::~MosquittoMqttPublisher(){

    if (mosq) {

        // mosquitto_disconnect() avisa al broker de forma ordenada (en vez
        // de simplemente cortar la conexión) - esto es lo que da tiempo
        // real a que un publish() pendiente (como el "OFFLINE" que se
        // manda justo antes de destruir la ventana) llegue a salir.
        mosquitto_disconnect(mosq);

        // mosquitto_loop_stop(mosq, false) para el hilo en segundo plano
        // de forma limpia. El "false" significa "no forzar" - espera a que
        // el hilo termine su trabajo actual en vez de matarlo a mitad.
        // Tiene que ir SIEMPRE antes de mosquitto_destroy() - destruir el
        // cliente con el hilo todavía vivo es la condición de carrera real
        // que comentamos arriba.
        mosquitto_loop_stop(mosq, false);

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