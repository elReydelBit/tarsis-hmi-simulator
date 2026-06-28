// MainWindow: main application window for the Tarsis HMI.
// Declared here (separate from .cpp) because moc requires the
// Q_OBJECT macro to be visible in a header file.

//This instruction warning the compiler to include this header file only once during compilation, 
//preventing duplicate definitions and errors.
#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include <QUdpSocket> // Include the QUdpSocket header for UDP communication
#include <QHostAddress> // Include the QHostAddress header for handling IP 
#include <QTcpSocket> // Include the QTcpSocket header for TCP connection

#include <QCloseEvent> // Full definition of QCloseEvent, needed to call event->accept()
#include <QTimer> // Needed for the alarm banner's blinking behavior


// Our own class: wraps a Mosquitto MQTT client used to announce
// this HMI's online/offline status.
#include "mosquittomqttpublisher.h" // Our own MQTT publisher class


class MainWindow : public QWidget {
    
      //Macro to enable Qt's meta-object features, such as signals and slots
      Q_OBJECT

    public:
        //         Default argument       
        MainWindow(QWidget * parent = nullptr);

    protected:
        // Qt calls this automatically right before the window closes.
        // We override it to publish "OFFLINE" before the program exits.
        void closeEvent(QCloseEvent *event) override;
    
    private:

        //Title and subtitle, centered at the top of the window
        QLabel *titleLabel=nullptr;
        QLabel *subtitleLabel=nullptr;

        //Connection status badges (TCP/UDP/MQTT)
        QLabel *tcpBadge=nullptr;
        QLabel *udpBadge=nullptr;
        QLabel *mqttBadge=nullptr;

        //Mission status banner (amber) - shown once RTL is actually sent
        QLabel *missionBanner=nullptr;

        //Critical alarm banner (red) - shown while battery is below
        //threshold, cleared only when the operator presses RTL
        QLabel *alarmBanner=nullptr;

        //Drives the blinking of alarmBanner while the alarm is active
        //and not yet acknowledged by the operator.
        QTimer *alarmBlinkTimer=nullptr;
        bool alarmBlinkOn=false;        // current blink phase
        bool alarmAcknowledged=false;   // true once RTL is pressed, until battery recovers

        // Polls mosqMqttPublisher.isConnected() once per second. Necesario
        // porque ese valor lo cambia un hilo de mosquitto, AJENO al sistema
        // de señales de Qt - sin este timer, nadie vuelve a preguntar nunca,
        // y el badge se queda congelado en lo que vio una sola vez al arrancar.
        QTimer *mqttPollTimer=nullptr;
        

        QLabel *label=nullptr;

        //Label telemetry data
        QLabel *altitudeLabel=nullptr;
        QLabel *speedLabel=nullptr;
        QLabel *batteryLabel=nullptr;

        QPushButton *buttonReconnet=nullptr;
        bool isUavOn=false;// Flag to track the current UAV status, toggled on each click


        //Button fot RTL order
        QPushButton *buttonRtl=nullptr;


        //Member fot UDP connection
        QUdpSocket *udpSocket=nullptr; // Pointer to the UDP socket for sending messages

        //Member for TCP connection
        QTcpSocket *tcpSocket= nullptr; //Pointer to the TCP socket 


        //Client for MQTT connection
        MosquittoMqttPublisher mosqMqttPublisher;


    private slots:
        
        //Slot function triggered when the Reconnect button is clicked 
        void onReconnectButtonClicked();

        
        //Slot function to handle incoming UDP data 
        void receiveUdpDatagram();  


        // Slot function triggered when the RTL button is clicked
        void onRtlButtonClicked();

        //Slot function to act when the TCP sockect connect to host
        void onTcpConnected();

        //Slot function when the Host sent info 
        void receiveTcpData();

        

        //Slot function when the connect having problem
        void onTcpError(QAbstractSocket :: SocketError socketError);

        //Slot function called by alarmBlinkTimer every 600ms
        void toggleAlarmBlink();

        //Slot function called by mqttPollTimer every 1000ms
        void checkMqttStatus();

};