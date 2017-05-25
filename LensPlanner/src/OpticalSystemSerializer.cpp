#include "OpticalSystemSerializer.h"

////////////////////////////////////////////////////////////////////////////////
OpticalSystemSerializer::OpticalSystemSerializer(QIODevice* io, 
    OLEF::OpticalSystem* system, ImageLibrary* library):
        m_imageLibrary(library),
        m_ioDevice(io),
        m_opticalSystem(system)
{}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemSerializer::serializeElement(QXmlStreamWriter& xml, const OLEF::OpticalSystemElement& element)
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
        xml.writeTextElement("mask", m_imageLibrary->lookUpTextureName(element.getTexture()));
    }
    if (element.getTextureFT() != 0)
    {
        xml.writeTextElement("maskFT", m_imageLibrary->lookUpTextureName(element.getTextureFT()));
    }

    xml.writeEndElement();
}

////////////////////////////////////////////////////////////////////////////////
bool OpticalSystemSerializer::serialize()
{
    // Create the xml stream.
    QXmlStreamWriter xml;
    xml.setDevice(m_ioDevice);
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
        serializeElement(xml, element);
    }
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void OpticalSystemSerializer::deserializeElement(QXmlStreamReader& xml, OLEF::OpticalSystemElement& element)
{
    if (xml.name() != "element")
    {
        xml.raiseError("Not a valid optical system file.");
        return;
    }

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
            else
            {
                xml.raiseError("Not a valid optical system file.");
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
            if (m_imageLibrary->loadImage(texturePath))
            {
                element.setTexture(m_imageLibrary->uploadTexture(texturePath));
            }
        }
        else if (xml.name() == "maskFT")
        {
            auto texturePath = xml.readElementText();
            if (m_imageLibrary->loadImage(texturePath))
            {
                element.setTextureFT(m_imageLibrary->uploadTexture(texturePath));
            }
        }
        else
        {
            xml.raiseError("Not a valid optical system file.");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool OpticalSystemSerializer::deserialize()
{
    // Resulting optical system.
    OLEF::OpticalSystem result;
    std::vector<OLEF::OpticalSystemElement> elements;

    // Create the xml stream.
    QXmlStreamReader xml;
    xml.setDevice(m_ioDevice);

    // Process the elements
    if (!xml.readNextStartElement() || xml.name() != "opticalSystem")
    {
        xml.raiseError("Not a valid optical system file.");
    }

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
                OLEF::OpticalSystemElement element;
                deserializeElement(xml, element);
                elements.push_back(element);
            }
        }
        else
        {
            xml.raiseError("Not a valid optical system file.");
        }
    }

    // Make sure it was successfuly read.
    if (xml.error())
        return false;

    // Save the results
    result.setElements(elements);
    *m_opticalSystem = result;

    return true;
}