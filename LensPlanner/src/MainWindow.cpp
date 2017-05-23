#include "MainWindow.h"

////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent)
{
    // Extract the desktop geometry.
    QRect desktopSize = QApplication::desktop()->availableGeometry();

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
        this, &MainWindow::loadOpticalSystem);

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
        this, &MainWindow::saveOpticalSystem);

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
        this, &MainWindow::generateGhostBounds);

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

    // Create the working optical system
    m_opticalSystem = new OLEF::OpticalSystem();

    // Initialize the main widgets.
    m_lensFlarePreviewer = new LensFlarePreviewer(m_opticalSystem, this);
    m_opticalSystemPreviewer = new OpticalSystemPreviewer(m_opticalSystem, this);
    m_opticalSystemEditor = new OpticalSystemEditor(
            m_lensFlarePreviewer->getImageLibrary(), m_opticalSystem, this);
    m_opticalSystemPreviewProperties = new OpticalSystemPreviewProperties(
        m_opticalSystemPreviewer, this);

    connect(m_opticalSystemEditor, &OpticalSystemEditor::opticalSystemChangedSignal, 
        m_opticalSystemPreviewer, &OpticalSystemPreviewer::opticalSystemChanged);
    connect(m_opticalSystemEditor, &OpticalSystemEditor::opticalSystemChangedSignal, 
        m_lensFlarePreviewer, &LensFlarePreviewer::opticalSystemChanged);
    connect(m_opticalSystemEditor, &OpticalSystemEditor::opticalSystemChangedSignal, 
        m_opticalSystemPreviewProperties, &OpticalSystemPreviewProperties::opticalSystemChanged);

    // Create the tab widget for the editor.
    QTabWidget* editorTab = new QTabWidget(this);
    editorTab->addTab(m_opticalSystemEditor, "Optical System");
    editorTab->addTab(m_opticalSystemPreviewProperties, "Optical System Preview");
    editorTab->addTab(new QWidget(this), "Lens Flare Preview");

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
MainWindow::~MainWindow()
{
    delete m_opticalSystem;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadOpticalSystem()
{
    // Browse a file.
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open Optical System"),
        //QDir::currentPath(),
        "D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems",
        QStringLiteral("Optical System Files (*.xml)"));

    // Make sure the dialog wasn't cancelled.
    if (fileName.isNull())
        return;

    // Open the file
    QFile fin(fileName);
    if (!fin.open(QIODevice::ReadOnly))
        return;

    // Resulting optical system.
    OLEF::OpticalSystem result;
    std::vector<OLEF::OpticalSystemElement> elements;

    // Create the xml stream.
    QXmlStreamReader xml;
    xml.setDevice(&fin);

    // Process the elements
    if (xml.readNextStartElement())
    {
        if (xml.name() == "opticalSystem")
        {
            while (xml.readNextStartElement())
            {
                if (xml.name() == "name")
                {
                    result.setName(xml.readElementText().toStdString());
                }
                else if (xml.name() == "fnumber")
                {
                    result.setFnumber(xml.readElementText().toDouble());
                }
                else if (xml.name() == "effectiveFocalLength")
                {
                    result.setEffectiveFocalLength(xml.readElementText().toDouble());
                }
                else if (xml.name() == "fieldOfView")
                {
                    result.setFieldOfView(xml.readElementText().toDouble());
                }
                else if (xml.name() == "filmWidth")
                {
                    result.setFilmWidth(xml.readElementText().toDouble());
                }
                else if (xml.name() == "filmHeight")
                {
                    result.setFilmHeight(xml.readElementText().toDouble());
                }
                else if (xml.name() == "elements")
                {
                    while (xml.readNextStartElement())
                    {
                        if (xml.name() == "element")
                        {
                            OLEF::OpticalSystemElement element;
                            while (xml.readNextStartElement())
                            {
                                if (xml.name() == "type")
                                {
                                    auto type = xml.readElementText();
                                    if (type == "lensSpherical")
                                    {
                                        element.setType(OLEF::OpticalSystemElement::ElementType::LENS_SPHERICAL);
                                    }
                                    else if (type == "lensAspherical")
                                    {
                                        element.setType(OLEF::OpticalSystemElement::ElementType::LENS_ASPHERICAL);
                                    }
                                    else if (type == "apertureStop")
                                    {
                                        element.setType(OLEF::OpticalSystemElement::ElementType::APERTURE_STOP);
                                    }
                                    else if (type == "sensor")
                                    {
                                        element.setType(OLEF::OpticalSystemElement::ElementType::SENSOR);
                                    }
                                }
                                else if (xml.name() == "height")
                                {
                                    element.setHeight(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "thickness")
                                {
                                    element.setThickness(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "radius")
                                {
                                    element.setRadiusOfCurvature(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "refractiveIndex")
                                {
                                    element.setIndexOfRefraction(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "abbeNumber")
                                {
                                    element.setAbbeNumber(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "coatingLambda")
                                {
                                    element.setCoatingLambda(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "coatingIor")
                                {
                                    element.setCoatingIor(xml.readElementText().toDouble());
                                }
                                else if (xml.name() == "mask")
                                {
                                    auto texturePath = xml.readElementText();
                                    if (m_lensFlarePreviewer->getImageLibrary()->loadImage(texturePath))
                                    {
                                        element.setTexture(m_lensFlarePreviewer->getImageLibrary()->uploadTexture(texturePath));
                                    }
                                }
                                else if (xml.name() == "maskFT")
                                {
                                    auto texturePath = xml.readElementText();
                                    if (m_lensFlarePreviewer->getImageLibrary()->loadImage(texturePath))
                                    {
                                        element.setTextureFT(m_lensFlarePreviewer->getImageLibrary()->uploadTexture(texturePath));
                                    }
                                }
                            }

                            elements.push_back(element);
                        }
                    }
                }
            }
        }
        else
        {
            xml.raiseError("Not a valid optical system file.");
        }
    }

    // Make sure we parsed it properly
    if (xml.error())
        return;

    // Save the results
    result.setElements(elements);
    *m_opticalSystem = result;

    // Let the views know about the change.
    m_opticalSystemEditor->opticalSystemChanged();
    m_opticalSystemPreviewer->opticalSystemChanged();
    m_lensFlarePreviewer->opticalSystemChanged();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadGhostBounds()
{
    // Browse a file.
    QString fileName = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open Optical System"),
        //QDir::currentPath(),
        "D:/Program/Programming/Projects/Cpp/OpenLensFlare/OpenLensFlare/examples/systems",
        QStringLiteral("Optical System Files (*.xml)"));

    // Make sure the dialog wasn't cancelled.
    if (fileName.isNull())
        return;

    // Open the file
    QFile fin(fileName);
    if (!fin.open(QIODevice::ReadOnly))
        return;
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::loadStarburst()
{
    // There is only one starburst algorithm right now, so this is safe
    OLEF::DiffractionStarburstAlgorithm* starburst =
        (OLEF::DiffractionStarburstAlgorithm*) m_lensFlarePreviewer->getStarburstAlgorithm();

    // Load an image.
    auto imagePath = m_lensFlarePreviewer->getImageLibrary()->loadImageDialog(
        this, "Load Starburst Image");

    // Try to load it.
    if (m_lensFlarePreviewer->getImageLibrary()->loadImage(imagePath))
    {
        starburst->setTexture(m_lensFlarePreviewer->getImageLibrary()->
            uploadTexture(imagePath));
    }
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveOpticalSystem()
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

    // Create the xml stream.
    QXmlStreamWriter xml;
    xml.setDevice(&fin);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("opticalSystem");

    xml.writeTextElement("name", QString::fromStdString(m_opticalSystem->getName()));
    xml.writeTextElement("fnumber", QString::number(m_opticalSystem->getFnumber()));
    xml.writeTextElement("effectiveFocalLength", QString::number(m_opticalSystem->getEffectiveFocalLength()));
    xml.writeTextElement("fieldOfView", QString::number(m_opticalSystem->getFieldOfView()));
    xml.writeTextElement("filmWidth", QString::number(m_opticalSystem->getFilmWidth()));
    xml.writeTextElement("filmHeight", QString::number(m_opticalSystem->getFilmHeight()));
    
    xml.writeStartElement("elements");
    for (const auto& element: m_opticalSystem->getElements())
    {
        xml.writeStartElement("element");
        switch (element.getType())
        {
            case OLEF::OpticalSystemElement::ElementType::LENS_SPHERICAL:
                xml.writeTextElement("type", "lensSpherical");
                break;
                
            case OLEF::OpticalSystemElement::ElementType::LENS_ASPHERICAL:
                xml.writeTextElement("type", "lensAspherical");
                break;

            case OLEF::OpticalSystemElement::ElementType::APERTURE_STOP:
                xml.writeTextElement("type", "apertureStop");
                break;
                
            case OLEF::OpticalSystemElement::ElementType::SENSOR:
                xml.writeTextElement("type", "sensor");
                break;
        }
        
        xml.writeTextElement("height", QString::number(element.getHeight()));
        xml.writeTextElement("thickness", QString::number(element.getThickness()));
        xml.writeTextElement("radius", QString::number(element.getRadiusOfCurvature()));
        xml.writeTextElement("refractiveIndex", QString::number(element.getIndexOfRefraction()));
        xml.writeTextElement("abbeNumber", QString::number(element.getAbbeNumber()));
        xml.writeTextElement("coatingLambda", QString::number(element.getCoatingLambda()));
        xml.writeTextElement("coatingIor", QString::number(element.getCoatingIor()));
        if (element.getTexture() != 0)
        {
            xml.writeTextElement("mask", m_lensFlarePreviewer->getImageLibrary()->lookUpTextureName(element.getTexture()));
        }
        if (element.getTextureFT() != 0)
        {
            xml.writeTextElement("maskFT", m_lensFlarePreviewer->getImageLibrary()->lookUpTextureName(element.getTextureFT()));
        }

        xml.writeEndElement();
    }
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();
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

    // Create the xml stream.
    QXmlStreamWriter xml;
    xml.setDevice(&fin);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("ghostBounds");

    xml.writeEndElement();
    xml.writeEndDocument();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::saveStarburst()
{
    // There is only one starburst algorithm right now, so this is safe
    OLEF::DiffractionStarburstAlgorithm* starburst =
        (OLEF::DiffractionStarburstAlgorithm*) m_lensFlarePreviewer->getStarburstAlgorithm();

    // Save the texture.
    m_lensFlarePreviewer->getImageLibrary()->loadImage(starburst->getTexture(), 
        true);
    m_lensFlarePreviewer->getImageLibrary()->saveImageDialog(this, 
        "Save Starburst Image", starburst->getTexture());
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::quit()
{
    qApp->quit();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateGhostBounds()
{
    // There is only one ghost algorithm right now, so this is safe
    OLEF::RayTraceGhostAlgorithm* ghost =
        (OLEF::RayTraceGhostAlgorithm*) m_lensFlarePreviewer->getGhostAlgorithm();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::generateStarburst()
{
    // There is only one starburst algorithm right now, so this is safe
    OLEF::DiffractionStarburstAlgorithm* starburst =
        (OLEF::DiffractionStarburstAlgorithm*) m_lensFlarePreviewer->getStarburstAlgorithm();

    // Generate starburst.
    starburst->generateTexture(2048, 2048, 390.0f, 780.0f, 5.0f);

    m_lensFlarePreviewer->update();
}

////////////////////////////////////////////////////////////////////////////////
void MainWindow::about()
{}
