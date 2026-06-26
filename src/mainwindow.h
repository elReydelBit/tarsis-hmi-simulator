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


class MainWindow : public QWidget {
    
      //Macro to enable Qt's meta-object features, such as signals and slots
      Q_OBJECT

    public:
        //         Default argument       
        MainWindow(QWidget * parent = nullptr);
    
    private:
        QLabel *label=nullptr;

        //Label telemetry data
        QLabel *altitudeLabel=nullptr;
        QLabel *speedLabel=nullptr;
        QLabel *batteryLabel=nullptr;

        QPushButton *button=nullptr;
        bool isUavOn=false;// Flag to track the current UAV status, toggled on each click


        //Button fot RTL order
        QPushButton *buttonRtl=nullptr;


        //Member fot UDP connection
        QUdpSocket *udpSocket=nullptr; // Pointer to the UDP socket for sending messages

        //Member for TCP connection
        QTcpSocket *tcpSocket= nullptr; //Pointer to the TCP socket 



    private slots:
        
        //Slot function to handle button click events  
        void onButtonClicked();

        
        //Slot function to handle incoming UDP data 
        void receiveUdpDatagram();  


        //
        void onTcpConnected();

        //
        void receiveTcpData();

        //
        void onRtlButtonClicked();

};