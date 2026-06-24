#include "mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent){

    //Set the window title
    setWindowTitle("Tarsis HMI Simulator");

    //Create a label
    this->label =new QLabel("Welcome to the Tarsis HMI Simulator", this);
    //Create a button
    this->button = new QPushButton("Click Me", this);

    //Create a vertical Layout and add the label and button to it
    QVBoxLayout * Layout = new QVBoxLayout(this);
    //Generate the link to the label
    Layout->addWidget(this->label);
    //Add elastic space between the label and button
    Layout->addStretch();
    //Generate the link to the button
    Layout->addWidget(this->button);

    this->isUavOn = false; // Initialize the UAV status flag to OFF

    //Connect the button's clicked signal to the onButtonClicked slot
    QObject::connect(this->button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);




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


