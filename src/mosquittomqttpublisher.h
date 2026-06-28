#pragma once

#include <mosquitto.h> // C API: struct mosquitto, mosquitto_new, mosquitto_connect, etc.
#include <string>
#include <atomic> // std::atomic - variable segura de leer/escribir desde dos hilos a la vez (detalle abajo)

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

         // POR QUÉ atomic y no un bool normal: esta variable se ESCRIBE desde
        // el hilo en segundo plano de mosquitto (dentro de onConnectCallback/
        // onDisconnectCallback, definidos en el .cpp) y se LEE desde el hilo
        // principal de Qt (dentro de isConnected(), que llama el HMI). Un
        // bool normal no garantiza qué ve un hilo de lo que escribió el otro.
        // std::atomic<bool> sí lo garantiza: cada escritura se hace visible
        // de inmediato al otro hilo, sin instante intermedio "a medio
        // escribir". Iremos a fondo con esto en la próxima sesión - por
        // ahora, trátalo como "la versión segura entre hilos de un bool".
        std::atomic<bool> connected{false};

    
    private:
        // Callbacks estáticos: la API en C de mosquitto necesita punteros a
        // función "planos" (sin "this" capturado, sin sintaxis de método de
        // clase) - por eso NO pueden ser métodos normales. El parámetro
        // "obj" es cómo recuperamos "this" dentro del callback (se lo
        // pasamos a mosquitto_new() en el .cpp, Pieza B) para poder tocar
        // el estado real del objeto desde ahí.
        static void onConnectCallback(struct mosquitto *mosq, void *obj, int rc);
        static void onDisconnectCallback(struct mosquitto *mosq, void *obj, int rc);

    public:
        MosquittoMqttPublisher (const std::string &clientId);
        bool publish(const std::string &payload);
        bool isConnected() const; // lets callers (e.g. the HMI badge) check the real status
        ~MosquittoMqttPublisher();
        


};