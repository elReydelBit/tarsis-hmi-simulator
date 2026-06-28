#include "mainwindow.h"
#include <QVBoxLayout>
#include <QNetworkDatagram>// Include the QNetworkDatagram header for handling incoming UDP datagrams
#include <QHBoxLayout> //Allow to desing better IU 
#include <spdlog/spdlog.h>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent), mosqMqttPublisher("tarsis-hmi"){

    //Set the window title
    setWindowTitle("Panel de Control");

    

    //Window background. "QWidget" targets this window itself; "QLabel"
    //sets a default text color for any label that does NOT have its own
    //setStyleSheet (titleLabel, badges, cards and buttons already have
    //their own, so they keep their colors - this only rescues "label",
    //which never had one and would turn invisible (black-on-dark) otherwise.
    this->setStyleSheet(
        "QWidget { background-color: #1b1f27; border: 1px solid #2a2f3a; }"
        "QLabel { color: #e8eaed; }");

    //Title and subtitle, centered above everything else
    this->titleLabel = new QLabel("Tarsis UAV — HMI simulator", this);
    this->titleLabel->setAlignment(Qt::AlignCenter);
    this->titleLabel->setStyleSheet("font-size: 20px; font-weight: 600; color: #f5f5f5;");

    this->subtitleLabel = new QLabel("Telemetry & command interface", this);
    this->subtitleLabel->setAlignment(Qt::AlignCenter);
    this->subtitleLabel->setStyleSheet("font-size: 13px; color: #9aa0a6;");


    //Badge styles - green for "ok", red for "not connected/error".
    //Declared once here, reused for all three badges below.
    QString badgeOk =
        "QLabel { background-color: #1c3a2a; color: #4ade80; "
        "border-radius: 10px; padding: 4px 12px; font-size: 12px; }";
    QString badgeError =
        "QLabel { background-color: #3a1c1c; color: #f87171; "
        "border-radius: 10px; padding: 4px 12px; font-size: 12px; }";

    this->tcpBadge = new QLabel("● TCP disconnected", this);
    this->tcpBadge->setStyleSheet(badgeError);   // starts red: connection not confirmed yet

    this->udpBadge = new QLabel("● UDP error", this);
    this->udpBadge->setStyleSheet(badgeError);   // overwritten once bind() result is known, below

    // Estado inicial honesto: probablemente "offline" en este instante
    // exacto, aunque Kali esté arriba - el handshake real con el broker
    // todavía no ha dado tiempo a completarse. El mqttPollTimer (más abajo)
    // es quien lo corrige en cuanto el callback confirme la conexión real.
    this->mqttBadge = new QLabel("● MQTT offline", this);
    this->mqttBadge->setStyleSheet(badgeError);

    //Create a label
    this->label =new QLabel("Welcome to the Tarsis HMI Simulator", this);
    this->label->setAlignment(Qt::AlignCenter);
    this->label->setStyleSheet("font-size: 12px; color: #9aa0a6;");

    //Card style shared by the three telemetry labels.
    QString cardStyle =
        "QLabel { background-color: #1e2330; color: #e8eaed; "
        "border: 1px solid #2a2f3a; border-radius: 10px; "
        "padding: 14px 18px; font-size: 14px; }";

    //Create 3 labels for telemetry data. Rich text (icon + label on top,
    //big value below) instead of plain "Altitude: N/A".
    this->altitudeLabel = new QLabel(this);
    this->altitudeLabel->setTextFormat(Qt::RichText);
    this->altitudeLabel->setText(
        "<div style='color:#9aa0a6; font-size:13px;'>&#8593; Altitude</div>"
        "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#e8eaed;'>N/A</span></div>");

    this->speedLabel = new QLabel(this);
    this->speedLabel->setTextFormat(Qt::RichText);
    this->speedLabel->setText(
        "<div style='color:#9aa0a6; font-size:13px;'>&#8635; Speed</div>"
        "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#e8eaed;'>N/A</span></div>");

    this->batteryLabel = new QLabel(this);
    this->batteryLabel->setTextFormat(Qt::RichText);
    this->batteryLabel->setText(
        "<div style='color:#9aa0a6; font-size:13px;'>&#9645; Battery</div>"
        "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#e8eaed;'>N/A</span></div>");

    this->altitudeLabel->setStyleSheet(cardStyle);
    this->speedLabel->setStyleSheet(cardStyle);
    this->batteryLabel->setStyleSheet(cardStyle);

    //Mission status banner (amber). Stays in the layout permanently with a
    //fixed height - "off" now means transparent background + empty text,
    //not hide()/show(), so the space below never jumps when it activates.
    this->missionBanner = new QLabel(this);
    this->missionBanner->setFixedHeight(64);
    this->missionBanner->setStyleSheet(
        "QLabel { background: transparent; border: none; padding: 12px 16px; }");

    //Critical alarm banner (red). Same fixed-height trick as missionBanner,
    //but a bit taller (72px instead of 64) - the "acknowledged" state adds
    //a line of text and this avoids it ever overflowing.
    this->alarmBanner = new QLabel(this);
    this->alarmBanner->setFixedHeight(72);
    this->alarmBanner->setWordWrap(true);
    this->alarmBanner->setStyleSheet(
        "QLabel { background: transparent; border: none; padding: 12px 16px; }");

    //Blink timer: fires every 600ms while the alarm is active and not yet
    //acknowledged. toggleAlarmBlink() (defined below) does the actual work.
    this->alarmBlinkTimer = new QTimer(this);
    QObject::connect(this->alarmBlinkTimer, &QTimer::timeout, this, &MainWindow::toggleAlarmBlink);

    // MQTT poll timer: every second, ask mosqMqttPublisher.isConnected()
    // and update the badge if needed. Starts running immediately - this
    // is what makes MQTT behave like TCP/UDP from the user's point of view,
    // even though under the hood it's polling, not a real Qt signal.
    this->mqttPollTimer = new QTimer(this);
    QObject::connect(this->mqttPollTimer, &QTimer::timeout, this, &MainWindow::checkMqttStatus);
    this->mqttPollTimer->start(1000);



    //Create a button
    this->buttonReconnet = new QPushButton("\u27f2 Reconnect", this);
    this->buttonReconnet->setFixedHeight(56);
    this->buttonReconnet->setMinimumWidth(130);
    this->buttonReconnet->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #454b59, stop:1 #2a2f3a); "
        "color: #e8eaed; border: 1px solid #5a6170; border-radius: 12px; padding: 10px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #525968, stop:1 #343a46); }"
        "QPushButton:pressed { background-color: #2a2f3a; }");

    //Create a RTL button
    this->buttonRtl= new QPushButton("\u21a9 RTL", this);

    //Create a vertical Layout and add the label and button to it
    QVBoxLayout * Layout = new QVBoxLayout(this);

    //Generate the link to the title and subtitle, added BEFORE the status label
    Layout->addWidget(this->titleLabel);
    Layout->addWidget(this->subtitleLabel);

    //Badges row: TCP / UDP / MQTT, centered as a group
    QHBoxLayout *badgesRow = new QHBoxLayout();
    badgesRow->addStretch();
    badgesRow->addWidget(this->tcpBadge);
    badgesRow->addWidget(this->udpBadge);
    badgesRow->addWidget(this->mqttBadge);
    badgesRow->addStretch();
    Layout->addLayout(badgesRow);

    //Telemetry cards row: the three labels side by side, equal width
    QHBoxLayout *telemetryRow = new QHBoxLayout();
    telemetryRow->addWidget(this->altitudeLabel, 1);
    telemetryRow->addWidget(this->speedLabel, 1);
    telemetryRow->addWidget(this->batteryLabel, 1);

    Layout->addLayout(telemetryRow);

    //Banners: full width, only one (or none) visible at a time
    Layout->addWidget(this->missionBanner);
    Layout->addWidget(this->alarmBanner);
    Layout->addSpacing(16);


    //RTL button: styled as a critical action, fixed square size,
    //pushed to the right with a dedicated horizontal layout
    this->buttonRtl->setFixedSize(90, 56);
    this->buttonRtl->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ef4444, stop:1 #b91c1c); "
        "color: white; border: 2px solid #7f1d1d; border-radius: 0px; }"
        "QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f87171, stop:1 #dc2626); }"
        "QPushButton:pressed { background-color: #991b1b; }");

    //Add elastic space, pushing the bottom row (RTL + Reconnect) down
    Layout->addStretch();

    //Bottom row: RTL pinned to the far left (deliberately awkward position,
    //agreed in design - never adjacent to Reconnect by default), a fixed
    //gap, then Reconnect taking the remaining width. Same row, but
    //visually distanced so a misclick can't hit the wrong button.
    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->addWidget(this->buttonRtl);
    bottomRow->addStretch();
    bottomRow->addWidget(this->label);
    bottomRow->addStretch();
    bottomRow->addWidget(this->buttonReconnet);
    Layout->addLayout(bottomRow);



    this->isUavOn = false; // Initialize the UAV status flag to OFF


    //Create a UDP socket for listening messages
    this->udpSocket = new QUdpSocket(this);

    // Bind the socket to any available address and port 5000.
    // bind() returns true/false immediately - real signal of whether
    // we're actually listening, not just an assumption.
    bool udpOk = this->udpSocket->bind(QHostAddress::Any, 5000);

    if (udpOk) {
        // bind() succeeding only means the socket is listening - it does
        // NOT mean Kali is actually sending anything yet. Stays red with
        // an honest label until the first real datagram arrives below.
        this->udpBadge->setText("● UDP no data");
    }
    // if bind() fails, udpBadge stays "● UDP error" (already set above)

    //Create a TCP socket for sending messages
    this->tcpSocket = new QTcpSocket(this);
    
    // Critical command channel: opened right away at startup,
    // not on-demand, since RTL must always be ready to send.
    tcpSocket->connectToHost(QHostAddress("192.168.1.131"), 6000);





    //Connect the buttonReconnect's clicked signal to the onReconnectButtonClicked slot
    QObject::connect(this->buttonReconnet, &QPushButton::clicked, this, &MainWindow::onReconnectButtonClicked);

    //Connect the RTL button's clicked signal to the onRtlButtonClicked
    QObject::connect(this->buttonRtl, &QPushButton::clicked, this, &MainWindow:: onRtlButtonClicked);

    //Connect the readyRead signal of the UDP socket to the receiveUdpDatagram slot
    QObject::connect(this->udpSocket, &QUdpSocket::readyRead, this, &MainWindow::receiveUdpDatagram);

    //Connect TCP socket signals to their slots
    QObject::connect(this->tcpSocket, &QTcpSocket::connected, this, &MainWindow::onTcpConnected);
    QObject::connect(this->tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::receiveTcpData);
    QObject::connect(this->tcpSocket, &QTcpSocket::errorOccurred, this, &MainWindow:: onTcpError);

    // Announce that the HMI is now running, once everything above is ready.
    mosqMqttPublisher.publish("ONLINE");

    //Minimum width matching the mockup's proportions - prevents the
    //badges/cards row from ever feeling cramped if resized smaller.
    this->setMinimumWidth(640);
}

void MainWindow::onReconnectButtonClicked(){

    
    if (this->tcpSocket->state() == QAbstractSocket:: UnconnectedState){

        this->label->setText("Reconnecting...");
        this->tcpSocket->connectToHost(QHostAddress("192.168.1.131"), 6000);

    }else{

        this->label->setText("Warning: TCP connected, it need not reconnect");
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

        // A real datagram just arrived - THIS is the actual proof Kali is
        // talking to us, not just "the socket is technically open".
        this->udpBadge->setStyleSheet(
            "QLabel { background-color: #1c3a2a; color: #4ade80; "
            "border-radius: 10px; padding: 4px 12px; font-size: 12px; }");
        this->udpBadge->setText("● UDP receiving");

        // Update each label ONCE, after all fields have been parsed —
        // not on every single loop iteration.
        this->altitudeLabel->setText(
            "<div style='color:#9aa0a6; font-size:13px;'>&#8593; Altitude</div>"
            "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#e8eaed;'>" + altitudeValue +
            "</span> <span style='font-size:12px; color:#9aa0a6;'>m</span></div>");

        this->speedLabel->setText(
            "<div style='color:#9aa0a6; font-size:13px;'>&#8635; Speed</div>"
            "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#e8eaed;'>" + speedValue +
            "</span> <span style='font-size:12px; color:#9aa0a6;'>km/h</span></div>");
        // batteryLabel handled below, inside the threshold if/else -
        // its value color depends on whether it's critical or not.

        // Convert the battery percentage from text to a number, so we can
        // compare it against the 10% critical threshold.
        int batteryPercent = batteryValue.toInt();
        

        // Visual state of the battery card follows the same 10% threshold
        // already used for logging below - same number, two consequences.
        if (batteryPercent < 10) {
            this->batteryLabel->setStyleSheet(
                "QLabel { background-color: #2a1414; color: #f87171; "
                "border: 1px solid #f87171; border-radius: 10px; "
                "padding: 14px 18px; font-size: 14px; font-weight: 600; }");
            this->batteryLabel->setText(
                "<div style='color:#f87171; font-size:13px;'>&#9645; Battery</div>"
                "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#f87171;'>" + batteryValue +
                "</span> <span style='font-size:12px; color:#f87171;'>%</span></div>");

            // Only refresh text/start the blink if the operator hasn't
            // already acknowledged this episode - while acknowledged, the
            // muted banner set by onRtlButtonClicked stays untouched.
            if (!this->alarmAcknowledged) {
                this->alarmBanner->setText(
                    "\u26a0 RTL crítica requerida\nBatería al " + batteryValue + "% \u00b7 alt "
                    + altitudeValue + " m \u00b7 vel " + speedValue + " km/h");

                if (!this->alarmBlinkTimer->isActive()) {
                    this->alarmBlinkOn = true;
                    this->alarmBlinkTimer->start(600);
                }
            }
        } else {
            this->batteryLabel->setStyleSheet(
                "QLabel { background-color: #1e2330; color: #e8eaed; "
                "border: 1px solid #2a2f3a; border-radius: 10px; "
                "padding: 14px 18px; font-size: 14px; }");
            this->batteryLabel->setText(
                "<div style='color:#9aa0a6; font-size:13px;'>&#9645; Battery</div>"
                "<div style='margin-top:4px;'><span style='font-size:24px; font-weight:600; color:#e8eaed;'>" + batteryValue +
                "</span> <span style='font-size:12px; color:#9aa0a6;'>%</span></div>");

            // Genuine recovery: stop blinking, clear the banner completely,
            // and reset acknowledgement so a FUTURE crossing starts a fresh
            // alarm cycle from scratch (not still "acknowledged" from before).
            this->alarmBlinkTimer->stop();
            this->alarmAcknowledged = false;
            this->alarmBanner->setStyleSheet(
                "QLabel { background: transparent; border: none; padding: 12px 16px; }");
            this->alarmBanner->setText("");
        }


        if(batteryPercent < 10){
            // Battery critical: every single reading below threshold gets
            // logged here, each line self-contained with the full snapshot -
            // useful to reconstruct exactly how long the UAV was critical
            // before the operator acted.
            spdlog::warn("Battery critical: {}% - RTL required (alt: {} m, speed: {} Km/h)", 
                          batteryPercent, altitudeValue.toStdString(), speedValue.toStdString(),
                          speedValue.toStdString());


        }else{

             spdlog::info("Telemetry nominal - alt: {} m, speed: {} km/h, battery: {}%",
                           altitudeValue.toStdString(), speedValue.toStdString(), batteryPercent);

        }

    }

}

void MainWindow::onRtlButtonClicked(){

    // The operator just acted - stop the blink and freeze the banner in a
    // muted "acknowledged" state (still visible, just no longer alarming).
    // It only fully resets when the battery genuinely recovers (see the
    // else branch in receiveUdpDatagram).
    this->alarmBlinkTimer->stop();
    this->alarmAcknowledged = true;
    QString alarmDetail = this->alarmBanner->text().section('\n', 1);
    this->alarmBanner->setStyleSheet(
        "QLabel { background-color: #2a2424; color: #b08a8a; "
        "border: 1px solid #4a3a3a; border-radius: 10px; "
        "padding: 12px 16px; }");
    this->alarmBanner->setText("\u2713 Atendido por operador\n" + alarmDetail);

    if (this->tcpSocket->state() == QAbstractSocket::ConnectedState) {
        
        QByteArray commad = "RTL\n";
        this->tcpSocket->write(commad);  
        this->label->setText("RTL sent..."); 
        // Operator's decision, logged once, at critical level - distinct
        // from the repeated "battery low" warnings above it in alarms.log.
        spdlog::critical("Operator executed RTL - {} ( {} , {})",
                          this->batteryLabel->text().toStdString(), this->altitudeLabel->text().toStdString(), this->speedLabel->text().toStdString());
        
        this->missionBanner->setStyleSheet(
            "QLabel { background-color: #3a2f0f; color: #fbbf24; "
            "border: 1px solid #fbbf24; border-radius: 10px; "
            "padding: 12px 16px; }");
        this->missionBanner->setText("\u21a9 RTL en curso \u2014 retornando a base");   // command actually went out

    }else{
        
        //Warning if the TCP socket dont connect to Host
        this->label->setText("Warming: TCP not connected");

    }

   
 
}


void MainWindow::onTcpConnected(){
    
    this->label->setText("TCP connected!!!");
    this->tcpBadge->setStyleSheet(
        "QLabel { background-color: #1c3a2a; color: #4ade80; "
        "border-radius: 10px; padding: 4px 12px; font-size: 12px; }");
    this->tcpBadge->setText("● TCP connected");

}

void MainWindow::receiveTcpData(){


    //read the port
    QByteArray data = this->tcpSocket->readAll();

    //format
     QString receivedMessage = QString::fromUtf8(data);

    //Close the connection
    this->tcpSocket->disconnectFromHost();

    //Test
    qDebug() << "TCP recieved: " << receivedMessage;
}

void MainWindow::onTcpError(QAbstractSocket :: SocketError socketError){


        this->label->setText("TCP error: " + this->tcpSocket->errorString());
        this->tcpBadge->setStyleSheet(
            "QLabel { background-color: #3a1c1c; color: #f87171; "
            "border-radius: 10px; padding: 4px 12px; font-size: 12px; }");
        this->tcpBadge->setText("● TCP error");


}

void MainWindow::checkMqttStatus()
{
    if (this->mosqMqttPublisher.isConnected()) {
        this->mqttBadge->setText("● MQTT online");
        this->mqttBadge->setStyleSheet(
            "QLabel { background-color: #1c3a2a; color: #4ade80; "
            "border-radius: 10px; padding: 4px 12px; font-size: 12px; }");
    } else {
        this->mqttBadge->setText("● MQTT offline");
        this->mqttBadge->setStyleSheet(
            "QLabel { background-color: #3a1c1c; color: #f87171; "
            "border-radius: 10px; padding: 4px 12px; font-size: 12px; }");
    }
}


void MainWindow::toggleAlarmBlink()
{
    this->alarmBlinkOn = !this->alarmBlinkOn;

    if (this->alarmBlinkOn) {
        this->alarmBanner->setStyleSheet(
            "QLabel { background-color: #2a1414; color: #f87171; "
            "border: 1px solid #f87171; border-radius: 10px; "
            "padding: 12px 16px; }");
    } else {
        this->alarmBanner->setStyleSheet(
            "QLabel { background: transparent; border: none; padding: 12px 16px; }");
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    // Announce that the HMI is shutting down, before the window
    // actually closes and this object gets destroyed.
    mosqMqttPublisher.publish("OFFLINE");

    // Let Qt continue with its normal close behavior.
    event->accept();
}
