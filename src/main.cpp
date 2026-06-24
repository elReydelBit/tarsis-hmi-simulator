#include <QApplication>
#include "mainwindow.h"



int main(int argc, char *argv[]){

    //Create the application object
    QApplication app(argc, argv);

    //Create the main window
    MainWindow window;

    //Show the window
    window.show();

    //Run the application event Loop
    return app.exec();
}