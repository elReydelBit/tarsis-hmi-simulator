#include <QApplication>
#include <QWidget>




int main(int argc, char *argv[]){

    //Create the application object
    QApplication app(argc, argv);

    //Create the main window
    QWidget window;

    //Set the window title
    window.setWindowTitle("Tarsis HMI Simulator");

    //Show the window
    window.show();

    //Run the application event Loop
    return app.exec();
}