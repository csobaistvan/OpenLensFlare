#include "MainWindow.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // Declare the Qt app
    QApplication app(argc, argv);

    // Create the main window and display it
    MainWindow mainWindow;
    mainWindow.showMaximized();

    // Run Qt's main loop
    return app.exec();
    return 0; // Keep it here, in case running the main loop needs to be disabled.
}