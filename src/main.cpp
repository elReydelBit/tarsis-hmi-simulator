#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>



int main(int argc, char *argv[]){

    //Create the application object
    QApplication app(argc, argv);

    //Create the main window
    QWidget window;

    //Set the window title
    window.setWindowTitle("Tarsis HMI Simulator");
    
    //Create a label 
    QLabel label("Welcome to the Tarsis HMI Simulator");
    //Create a button 
    QPushButton button("Click Me");


    //Create a vertical Layout and add the label and button to it
    QVBoxLayout *layout = new QVBoxLayout(&window);
    //Generate the link to the label 
    layout->addWidget(&label);
    //Add elastic space between the label and button
    layout->addStretch();
    //Generate the link to the button
    layout->addWidget(&button);

    

    //Flag to track the current UAV status, toggled on each click
    bool flag = false;
    QObject::connect(&button, &QPushButton::clicked, [&]{
        
        if (!flag){

            label.setText("UAV STATUS: ON");
            flag =true;

        }else{

            label.setText("UAV STATUS: OFF");
            flag=false;

        }
    
        
    });


    //Show the window
    window.show();

    //Run the application event Loop
    return app.exec();
}