#pragma once

#include "Dependencies.h"

/// Helper object used for serializing ghost attributes.
class GhostSerializer
{
public:
    GhostSerializer(QIODevice* io, QMap<float, OLEF::GhostList>* ghosts);
    
    /// Serializes the underlying ghost attributes.
    bool serialize();

    /// Deserializes the underlying ghost attributes
    bool deserialize();

private:
    /// Serializes a ghost.
    void serializeGhost(QXmlStreamWriter& xml, const OLEF::Ghost& ghost);

    /// Serializes a ghost list.
    void serializeGhostList(QXmlStreamWriter& xml, const OLEF::GhostList& ghosts);

    /// Deserializes a ghost.
    void deserializeGhost(QXmlStreamReader& xml, OLEF::Ghost& ghost);

    /// Deserializes a ghost list.
    void deserializeGhostList(QXmlStreamReader& xml, float& angle, OLEF::GhostList& ghosts);

    /// Xml source/destination file.
    QIODevice* m_ioDevice;

    /// Ghost list to serialize/deserialize.
    QMap<float, OLEF::GhostList>* m_ghosts;
};