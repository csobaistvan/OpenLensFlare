#pragma once

#include "ImageLibrary.h"
#include "OpticalSystemEditor.h"
#include "OpticalSystemPreviewer.h"
#include "LensFlarePreviewer.h"
#include "OpticalSystemPreviewProperties.h"
#include "LensFlarePreviewProperties.h"
#include "OpticalSystemSerializer.h"
#include "GhostSerializer.h"

#include <QMainWindow>

class MainWindow: public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
      void showEvent(QShowEvent *ev);
      
private:
    void createMenu();
    void createLensFlareObjects();
    void setupLayout();
    void setupTestScene();

    void loadOpticalSystem(const QString& fileName);

    /// File menu callbacks
    void loadSystem();
    void loadGhostBounds();
    void loadStarburst();
    void saveSystem();
    void saveGhostBounds();
    void saveStarburst();
    void quit();

    /// Generate menu callbacks
    void generateStarburst();
    void generateBounds();

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

    /// Editor object for the optical system preview options.
    OpticalSystemPreviewProperties* m_opticalSystemPreviewProperties;

    /// Editor object for the lens flare preview options.
    LensFlarePreviewProperties* m_lensFlarePreviewProperties;

    /// The folder where the browse window should open.
    QString m_browseFolder;
};