#include "GhostSerializer.h"

////////////////////////////////////////////////////////////////////////////////
GhostSerializer::GhostSerializer(QIODevice* io, QMap<float, OLEF::GhostList>* ghosts):
    m_ioDevice(io),
    m_ghosts(ghosts)
{}

////////////////////////////////////////////////////////////////////////////////
void GhostSerializer::serializeGhost(QXmlStreamWriter& xml, const OLEF::Ghost& ghost)
{
    // Serialize the interfaces
    xml.writeStartElement("interfaces");

    //for (auto interface: ghost)
    for (auto it = ghost.begin(); it != ghost.end(); ++it)
    {
        xml.writeTextElement("interface", QString::number(*it));
    }

    xml.writeEndElement();
    
    // Serialize the pupil bounds
    xml.writeStartElement("pupilBounds");
    
    xml.writeTextElement("x", QString::number(ghost.getPupilBounds()[0].x));
    xml.writeTextElement("y", QString::number(ghost.getPupilBounds()[0].y));
    xml.writeTextElement("w", QString::number(ghost.getPupilBounds()[1].x));
    xml.writeTextElement("h", QString::number(ghost.getPupilBounds()[1].y));

    xml.writeEndElement();
    
    // Serialize the sensor bounds
    xml.writeStartElement("sensorBounds");
    
    xml.writeTextElement("x", QString::number(ghost.getSensorBounds()[0].x));
    xml.writeTextElement("y", QString::number(ghost.getSensorBounds()[0].y));
    xml.writeTextElement("w", QString::number(ghost.getSensorBounds()[1].x));
    xml.writeTextElement("h", QString::number(ghost.getSensorBounds()[1].y));

    xml.writeEndElement();

    // Serialize the rest of the attributes
    xml.writeTextElement("avgIntensity", QString::number(ghost.getAverageIntensity()));
    xml.writeTextElement("minChannels", QString::number(ghost.getMinimumChannels()));
    xml.writeTextElement("optimalChannels", QString::number(ghost.getOptimalChannels()));
    xml.writeTextElement("minRays", QString::number(ghost.getMinimumRays()));
    xml.writeTextElement("optimalRays", QString::number(ghost.getOptimalRays()));
}

////////////////////////////////////////////////////////////////////////////////
void GhostSerializer::serializeGhostList(QXmlStreamWriter& xml, const OLEF::GhostList& ghosts)
{
    xml.writeStartElement("ghosts");

    for (const auto& ghost: ghosts)
    {
        xml.writeStartElement("ghost");

        serializeGhost(xml, ghost);

        xml.writeEndElement();
    }

    xml.writeEndElement();
}

////////////////////////////////////////////////////////////////////////////////
bool GhostSerializer::serialize()
{
    // Create the xml stream.
    QXmlStreamWriter xml;
    xml.setDevice(m_ioDevice);
    xml.setAutoFormatting(true);

    xml.writeStartDocument();
    xml.writeStartElement("ghostList");

    for (auto it = m_ghosts->begin(); it != m_ghosts->end(); ++it)
    {
        xml.writeStartElement("list");

        xml.writeTextElement("angle", QString::number(it.key()));
        serializeGhostList(xml, it.value());
        
        xml.writeEndElement();
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    return true;
}

////////////////////////////////////////////////////////////////////////////////
void GhostSerializer::deserializeGhost(QXmlStreamReader& xml, OLEF::Ghost& ghost)
{
    if (xml.name() != "ghost")
    {
        xml.raiseError("Not a valid ghost list file.");
        return;
    }

    while (xml.readNextStartElement())
    {
        if (xml.name() == "interfaces")
        {
            int ifaceId = 0;
            while (xml.readNextStartElement())
            {
                if (xml.name() == "interface")
                {
                    ghost[ifaceId++] = xml.readElementText().toInt();
                }
                else
                {
                    xml.raiseError("Not a valid ghost list file.");
                    return;
                }
            }
            ghost.setLength(ifaceId);
        }
        else if (xml.name() == "pupilBounds")
        {
            OLEF::Ghost::BoundingRect bounds;
            while (xml.readNextStartElement())
            {
                if (xml.name() == "x")
                {
                    bounds[0].x = xml.readElementText().toDouble();
                }
                else if (xml.name() == "y")
                {
                    bounds[0].y = xml.readElementText().toDouble();
                }
                else if (xml.name() == "w")
                {
                    bounds[1].x = xml.readElementText().toDouble();
                }
                else if (xml.name() == "h")
                {
                    bounds[1].y = xml.readElementText().toDouble();
                }
                else
                {
                    xml.raiseError("Not a valid ghost list file.");
                    return;
                }
            }
            ghost.setPupilBounds(bounds);
        }
        else if (xml.name() == "sensorBounds")
        {
            OLEF::Ghost::BoundingRect bounds;
            while (xml.readNextStartElement())
            {
                if (xml.name() == "x")
                {
                    bounds[0].x = xml.readElementText().toDouble();
                }
                else if (xml.name() == "y")
                {
                    bounds[0].y = xml.readElementText().toDouble();
                }
                else if (xml.name() == "w")
                {
                    bounds[1].x = xml.readElementText().toDouble();
                }
                else if (xml.name() == "h")
                {
                    bounds[1].y = xml.readElementText().toDouble();
                }
                else
                {
                    xml.raiseError("Not a valid ghost list file.");
                    return;
                }
            }
            ghost.setSensorBounds(bounds);
        }
        else if (xml.name() == "avgIntensity")
        {
            ghost.setAverageIntensity(xml.readElementText().toDouble());
        }
        else if (xml.name() == "minChannels")
        {
            ghost.setMinimumChannels(xml.readElementText().toDouble());
        }
        else if (xml.name() == "optimalChannels")
        {
            ghost.setOptimalChannels(xml.readElementText().toDouble());
        }
        else if (xml.name() == "minRays")
        {
            ghost.setMinimumRays(xml.readElementText().toDouble());
        }
        else if (xml.name() == "optimalRays")
        {
            ghost.setOptimalRays(xml.readElementText().toDouble());
        }
        else
        {
            xml.raiseError("Not a valid ghost list file.");
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void GhostSerializer::deserializeGhostList(QXmlStreamReader& xml, float& angle, OLEF::GhostList& ghosts)
{
    if (xml.name() != "list")
    {
        xml.raiseError("Not a valid ghost list file.");
        return;
    }

    while (xml.readNextStartElement())
    {
        if (xml.name() == "angle")
        {
            angle = xml.readElementText().toDouble();
        }
        else if (xml.name() == "ghosts")
        {
            while (xml.readNextStartElement())
            {
                OLEF::Ghost ghost;
                deserializeGhost(xml, ghost);
                ghosts.push_back(ghost);
            }
        }
        else
        {
            xml.raiseError("Not a valid ghost list file.");
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool GhostSerializer::deserialize()
{
    // Resulting optical system.
    QMap<float, OLEF::GhostList> result;

    // Create the xml stream.
    QXmlStreamReader xml;
    xml.setDevice(m_ioDevice);

    // Process the elements
    if (!xml.readNextStartElement() || xml.name() != "ghostList")
    {
        xml.raiseError("Not a valid ghost list file.");
    }

    while (xml.readNextStartElement())
    {
        OLEF::GhostList list;
        float angle;
        deserializeGhostList(xml, angle, list);
        result[angle] = list;
    }

    // Make sure it was successfuly read.
    if (xml.error())
    {
        return false;
    }

    // Save the results
    *m_ghosts = result;

    return true;
}