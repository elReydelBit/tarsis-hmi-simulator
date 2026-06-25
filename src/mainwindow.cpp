#include "mainwindow.h"
#include <QVBoxLayout>
#include <QNetworkDatagram>// Include the QNetworkDatagram header for handling incoming UDP datagrams

MainWindow::MainWindow(QWidget *parent) : QWidget(parent){

    //Set the window title
    setWindowTitle("Tarsis HMI Simulator");

    //Create a label
    this->label =new QLabel("Welcome to the Tarsis HMI Simulator", this);

    //Create 3 labels for telemetry data
    this->altitudeLabel = new QLabel("Altitude: N/A", this);
    this->speedLabel = new QLabel("Speed: N/A",this);
    this->batteryLabel = new QLabel("Battery: N/A", this);



    //Create a button
    this->button = new QPushButton("Click Me", this);

    //Create a vertical Layout and add the label and button to it
    QVBoxLayout * Layout = new QVBoxLayout(this);
    //Generate the link to the label
    Layout->addWidget(this->label);

    //Generate the link to the telemetry labels
    Layout->addWidget(this->altitudeLabel);
    Layout->addWidget(this->speedLabel);
    Layout->addWidget(this->batteryLabel);


    //Add elastic space between the label and button
    Layout->addStretch();


    //Generate the link to the button
    Layout->addWidget(this->button);

    this->isUavOn = false; // Initialize the UAV status flag to OFF


    //Create a UDP socket for sending messages
    this->udpSocket = new QUdpSocket(this);

    // Bind the socket to any available address and port 5000
    this->udpSocket->bind(QHostAddress::Any, 5000); 

    //Connect the button's clicked signal to the onButtonClicked slot
    QObject::connect(this->button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);

    //Connect the readyRead signal of the UDP socket to the receiveUdpDatagram slot
    QObject::connect(this->udpSocket, &QUdpSocket::readyRead, this, &MainWindow::receiveUdpDatagram);




}

void MainWindow::onButtonClicked(){


    //Turn on or off the flag
    this->isUavOn = !this->isUavOn;

    //Update the text label based on the current UAV status
    if(this->isUavOn){

        this->label->setText("UAV status: ON");
       

    }else{
        
        this->label->setText("UAV status: OFF");
        
    }


}

void MainWindow::receiveUdpDatagram(){
    
    // Check if at least one UDP datagram is waiting to be read.
    // readyRead() only notifies that something arrived; it doesn't
    // hand us the data automatically — we have to go fetch it.

    if (this->udpSocket->hasPendingDatagrams()){

        // Fetch the full datagram: raw bytes + metadata (e.g. sender IP).
        QNetworkDatagram datagram = this->udpSocket->receiveDatagram();
    
        // Extract only the raw bytes from the datagram, ignoring metadata.
        QByteArray payload = datagram.data();

        // Bytes are not text by themselves — explicitly decode them
        // as UTF-8 characters to get a readable QString.
        QString receivedMessage = QString::fromUtf8(payload);

        // Split the full message by comma, separating the three
        // "KEY:VALUE" fields (e.g. "ALT:87", "SPD:49", "BAT:67").
        QStringList parts = receivedMessage.split(",");
        
        // Auxiliary variables to hold each parsed value, identified
        // by their key name — NOT by their position in the list.
        QString altitudeValue;
        QString speedValue;
        QString batteryValue;


        // Loop over each "KEY:VALUE" chunk we got from the split above.
        // 'const QString &part' avoids copying each element.
        for(const QString &part : parts ){

            // Split this single chunk by colon: "ALT:87" -> ["ALT", "87"]
            QStringList keyValue = part.split(":");
            
            // Safety check: if this chunk doesn't have exactly a key
            // AND a value (e.g. corrupted/incomplete data), skip it
            // instead of crashing when accessing index 1 below.
            if (keyValue.size() < 2){
                continue;// jump straight to the next loop iteration
            }

            // Now it's safe to read both parts.
            QString key = keyValue[0];
            QString value = keyValue[1];



            // Match the key explicitly, so the order in which fields
            // arrive in the message never matters.
            if (key == "ALT"){
                altitudeValue=value;
            }else if(key == "SPD"){
                speedValue=value;
            }else if(key == "BAT"){
                batteryValue=value;
            }


        }

        // Update each label ONCE, after all fields have been parsed —
        // not on every single loop iteration.
        this->altitudeLabel->setText("Altitude: "+ altitudeValue +" m");
        this->speedLabel->setText("Speed: "+ speedValue +" Km/h");
        this->batteryLabel->setText("Battery: "+ batteryValue +" %");


    }

}

