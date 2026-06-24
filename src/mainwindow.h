// MainWindow: main application window for the Tarsis HMI.
// Declared here (separate from .cpp) because moc requires the
// Q_OBJECT macro to be visible in a header file.

//This instruction warning the compiler to include this header file only once during compilation, 
//preventing duplicate definitions and errors.
#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>


class MainWindow : public QWidget {
    
      //Macro to enable Qt's meta-object features, such as signals and slots
      Q_OBJECT

    public:
        //         Default argument       
        MainWindow(QWidget * parent = nullptr);
    
    private:
        QLabel *label=nullptr;
        QPushButton *button=nullptr;
        bool isUavOn=false;// Flag to track the current UAV status, toggled on each click


    private slots:
        void onButtonClicked();
};