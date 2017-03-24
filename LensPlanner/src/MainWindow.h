#pragma once

#include "ImageLibrary.h"
#include "OpticalSystemEditor.h"
#include "OpticalSystemPreviewer.h"
#include "LensFlarePreviewer.h"

#include <QMainWindow>

class MainWindow: public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    /// File menu callbacks
    void loadOpticalSystem();
    void loadGhostBounds();
    void loadStarburst();
    void saveOpticalSystem();
    void saveGhostBounds();
    void saveStarburst();
    void quit();

    /// Generate menu callbacks
    void generateGhostBounds();
    void generateStarburst();

    /// Help menu callbacks
    void about();

    /// The optical system that is worked with.
    OLEF::OpticalSystem* m_opticalSystem;

    /// Pointer to the optical system editor widget.
    OpticalSystemEditor* m_opticalSystemEditor;

    /// Pointer to the optical system previewer object.
    OpticalSystemPreviewer* m_opticalSystemPreviewer;

    /// Pointer to the lens flare previewer object.
    LensFlarePreviewer* m_lensFlarePreviewer;
};