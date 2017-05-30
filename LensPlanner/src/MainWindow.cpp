#include "MainWindow.h"

////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    m_browseFolder(QDir::currentPath())
{
    createLensFlareObjects();
    createMenu();
    setupLayout();
}

////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
    delete m_opticalSystem;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::showEvent(QShowEvent *ev)
{
    QMainWindow::showEvent(ev);
    setupTestScene();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::createLensFlareObjects()
{
    // Create the working optical system
    m_opticalSystem = new OLEF::OpticalSystem();

    // Initialize the main widgets.
    m_lensFlarePreviewer = new LensFlarePreviewer(
        m_opticalSystem, this);
    m_opticalSystemPreviewer = new OpticalSystemPreviewer(
        m_opticalSystem, this);
    m_opticalSystemEditor = new OpticalSystemEditor(
        m_lensFlarePreviewer->getImageLibrary(), m_opticalSystem, this);
    m_opticalSystemPreviewProperties = new OpticalSystemPreviewProperties(
        m_opticalSystemPreviewer, this);
    m_lensFlarePreviewProperties = new LensFlarePreviewProperties(
        m_lensFlarePreviewer, this);

    connect(m_opticalSystemEditor, 
        &OpticalSystemEditor::opticalSystemChangedSignal, 
        m_opticalSystemPreviewer, 
        &OpticalSystemPreviewer::opticalSystemChanged);
    connect(m_opticalSystemEditor, 
        &OpticalSystemEditor::opticalSystemChangedSignal, 
        m_lensFlarePreviewer, 
        &LensFlarePreviewer::opticalSystemChanged);
    connect(m_opticalSystemEditor, 
        &OpticalSystemEditor::opticalSystemChangedSignal, 
        m_opticalSystemPreviewProperties, 
        &OpticalSystemPreviewProperties::opticalSystemChanged);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::setupLayout()
{
    // Extract the desktop geometry.
    QRect desktopSize = QApplication::desktop()->availableGeometry();

    // Create the tab widget for the editor.
    QTabWidget* editorTab = new QTabWidget(this);
    editorTab->addTab(m_opticalSystemEditor, "Optical System");
    editorTab->addTab(m_opticalSystemPreviewProperties, "Optical System Preview");
    editorTab->addTab(m_lensFlarePreviewProperties, "Lens Flare Preview");

    // Create the top splitter, which contains the editor and lens flare widgets.
    QSplitter* topSplitter = new QSplitter(Qt::Horizontal, this);
    topSplitter->addWidget(editorTab);
    topSplitter->addWidget(m_lensFlarePreviewer);
    topSplitter->setSizes
    ({
        m_opticalSystemEditor->minimumSize().width(),
        desktopSize.width() - m_opticalSystemEditor->minimumSize().width(),
    });

    // Create the horizontal splitter which splits the top section (editors and
    // lens flare preview) and the optical system previewer widgets.
    QSplitter* horizontalSplitter = new QSplitter(Qt::Vertical, this);
    horizontalSplitter->addWidget(topSplitter);
    horizontalSplitter->addWidget(m_opticalSystemPreviewer);
    horizontalSplitter->setSizes
    ({
        desktopSize.height() - m_opticalSystemPreviewer->minimumSize().height(),
        m_opticalSystemPreviewer->minimumSize().height(),
    });

    // Set the horizontal splitter as the central widget
    setCentralWidget(horizontalSplitter);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::createMenu()
{
    // Create the menu bar
    QMenuBar* menuBar = new QMenuBar;
    setMenuBar(menuBar);

    // File menu
    QMenu* fileMenu = new QMenu;
    menuBar->addMenu(fileMenu);
    fileMenu->setTitle("File");

    // File->load optical system
    QAction* loadSystemAction = new QAction;
    fileMenu->addAction(loadSystemAction);
    loadSystemAction->setText("Load Optical System...");
    connect(loadSystemAction, &QAction::triggered, 
        this, &MainWindow::loadSystem);

    // File->load ghost bounds
    QAction* loadBoundsAction = new QAction;
    fileMenu->addAction(loadBoundsAction);
    loadBoundsAction->setText("Load Ghost Bounds...");
    connect(loadBoundsAction, &QAction::triggered, 
        this, &MainWindow::loadGhostBounds);

    // File->load starburst texture
    QAction* loadStarburstAction = new QAction;
    fileMenu->addAction(loadStarburstAction);
    loadStarburstAction->setText("Load Starburst...");
    connect(loadStarburstAction, &QAction::triggered, 
        this, &MainWindow::loadStarburst);

    fileMenu->addSeparator();

    // File->save optical system
    QAction* saveSystemAction = new QAction;
    fileMenu->addAction(saveSystemAction);
    saveSystemAction->setText("Save Optical System...");
    connect(saveSystemAction, &QAction::triggered, 
        this, &MainWindow::saveSystem);

    // File->save ghost bounds
    QAction* saveBoundsAction = new QAction;
    fileMenu->addAction(saveBoundsAction);
    saveBoundsAction->setText("Save Ghost Bounds...");
    connect(saveBoundsAction, &QAction::triggered, 
        this, &MainWindow::saveGhostBounds);

    // File->save starburst texture
    QAction* saveStarburstAction = new QAction;
    fileMenu->addAction(saveStarburstAction);
    saveStarburstAction->setText("Save Starburst...");
    connect(saveStarburstAction, &QAction::triggered, 
        this, &MainWindow::saveStarburst);

    fileMenu->addSeparator();

    // File->quit
    QAction* quitAction = new QAction;
    fileMenu->addAction(quitAction);
    quitAction->setText("Quit");
    connect(quitAction, &QAction::triggered, 
        this, &MainWindow::quit);

    // Generate menu
    QMenu* generateMenu = new QMenu;
    menuBar->addMenu(generateMenu);
    generateMenu->setTitle("Generate");

    // Generate->generate ghost bounds
    QAction* generateBoundsAction = new QAction;
    generateMenu->addAction(generateBoundsAction);
    generateBoundsAction->setText("Generate Ghost Bounds");
    connect(generateBoundsAction, &QAction::triggered, 
        this, &MainWindow::generateBounds);

    // Generate->generate starburst texture
    QAction* generateStarburstAction = new QAction;
    generateMenu->addAction(generateStarburstAction);
    generateStarburstAction->setText("Generate Starburst");
    connect(generateStarburstAction, &QAction::triggered, 
        this, &MainWindow::generateStarburst);

    // Help menu
    QMenu* helpMenu = new QMenu;
    menuBar->addMenu(helpMenu);
    helpMenu->setTitle("Help");

    // Help->about
    QAction* aboutAction = new QAction;
    helpMenu->addAction(aboutAction);
    aboutAction->setText("About");
    connect(aboutAction, &QAction::triggered, 
        this, &MainWindow::about);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::setupTestScene()
{
    m_browseFolder = "D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems";
    
    QString aperture = m_browseFolder + "/apertureDist.bmp";
    QString apertureFT = m_browseFolder + "/apertureFT.bmp";
    ImageLibrary* imgLibrary = m_lensFlarePreviewer->getImageLibrary();

    loadOpticalSystem(m_browseFolder + "/heliar-tronnier.xml");
    imgLibrary->loadImage(aperture);
    imgLibrary->loadImage(apertureFT);
    (*m_opticalSystem)[5].setTexture(imgLibrary->uploadTexture(aperture));
    (*m_opticalSystem)[5].setTextureFT(imgLibrary->uploadTexture(apertureFT));
    m_opticalSystemEditor->update();

    m_lensFlarePreviewer->requestStarburstGeneration();
    m_lensFlarePreviewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadOpticalSystem(const QString& fileName)
{
    // Open the file
    QFile fin(fileName);
    if (!fin.open(QIODevice::ReadOnly))
        return;

    // Deserialize the object.
    OpticalSystemSerializer serializer(&fin, m_opticalSystem, 
        m_lensFlarePreviewer->getImageLibrary());
    if (!serializer.deserialize())
        return;

    // Let the views know about the change.
    m_opticalSystemEditor->opticalSystemChanged();
    m_opticalSystemPreviewer->opticalSystemChanged();
    m_lensFlarePreviewer->opticalSystemChanged();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadSystem()
{
    // Browse a file.
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open Optical System"),
        m_browseFolder,
        QStringLiteral("Optical System Files (*.xml)"));

    // Make sure the dialog wasn't cancelled.
    if (!fileName.isNull())
        loadOpticalSystem(fileName);
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadGhostBounds()
{
    // Browse a file.
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open Optical System"),
        m_browseFolder,
        QStringLiteral("Optical System Files (*.xml)"));

    // Make sure the dialog wasn't cancelled.
    if (fileName.isNull())
        return;

    // Open the file
    QFile fin(fileName);
    if (!fin.open(QIODevice::ReadOnly))
        return;

    // Deserialize the object.
    QMap<float, OLEF::GhostList> ghosts;
    GhostSerializer serializer(&fin, &ghosts);
    if (!serializer.deserialize())
        return;

    // Store the deserialized ghosts
    m_lensFlarePreviewer->setPrecomputedGhosts(ghosts);

    // Let the views know about the change.
    m_lensFlarePreviewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadStarburst()
{
    // Create a browse dialog.
    auto imagePath = m_lensFlarePreviewer->getImageLibrary()->loadImageDialog(
        this, "Load Starburst Image");

    // Store it in the render object.
    m_lensFlarePreviewer->getDiffractionStarburstAlgorithm()->setTexture(
        m_lensFlarePreviewer->getImageLibrary()->uploadTexture(imagePath));
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveSystem()
{
    // Browse a file.
    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Save Optical System"),
        //QDir::currentPath(),
        "D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems",
        QStringLiteral("Optical System Files (*.xml)"));

    // Make sure the dialog wasn't cancelled.
    if (fileName.isNull())
        return;

    // Open the file
    QFile fin(fileName);
    if (!fin.open(QIODevice::WriteOnly))
        return;

    // Serialize the object.
    OpticalSystemSerializer serializer(
        &fin, m_opticalSystem, m_lensFlarePreviewer->getImageLibrary());
    serializer.serialize();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveGhostBounds()
{
    // Browse a file.
    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Save Ghost Bounds"),
        //QDir::currentPath(),
        "D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems",
        QStringLiteral("Ghost Bounds Files (*.xml)"));

    // Make sure the dialog wasn't cancelled.
    if (fileName.isNull())
        return;

    // Open the file
    QFile fin(fileName);
    if (!fin.open(QIODevice::WriteOnly))
        return;

    // Serialize the object.
    auto ghosts = m_lensFlarePreviewer->getPrecomputedGhosts();
    GhostSerializer serializer(&fin, &ghosts);
    serializer.serialize();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveStarburst()
{
    // Load back the generated texture from the GPU into main memory.
    m_lensFlarePreviewer->getImageLibrary()->loadImage(
        m_lensFlarePreviewer->getDiffractionStarburstAlgorithm()->getTexture(),
         true);

    // Create a save image dialog and save the texture.
    m_lensFlarePreviewer->getImageLibrary()->saveImageDialog(
        this, "Save Starburst Image", 
        m_lensFlarePreviewer->getDiffractionStarburstAlgorithm()->getTexture());
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::quit()
{
    qApp->quit();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::about()
{}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateStarburst()
{
    m_lensFlarePreviewer->requestStarburstGeneration();
    m_lensFlarePreviewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateBounds()
{
    m_lensFlarePreviewer->requestPrecomputation();
    m_lensFlarePreviewer->update();
}