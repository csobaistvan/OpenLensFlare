#pragma once

#include "Dependencies.h"
#include "ImageLibrary.h"

/// Helper object used for optical system serialization/deserialization.
class OpticalSystemSerializer
{
public:
    OpticalSystemSerializer(QIODevice* io, OLEF::OpticalSystem* system, ImageLibrary* library);
    
    /// Serializes the underlying optical system.
    bool serialize();

    /// Deserializes an optical system into the underlying object.
    bool deserialize();

private:
    /// Serializes an optical system element.
    void serializeElement(QXmlStreamWriter& xml, const OLEF::OpticalSystemElement& element);

    /// Deserializes an optical system element.
    void deserializeElement(QXmlStreamReader& xml, OLEF::OpticalSystemElement& element);

    /// Xml source/destination file.
    QIODevice* m_ioDevice;

    /// The target optical system.
    OLEF::OpticalSystem* m_opticalSystem;

    /// The image library, used for uploading textures.
    ImageLibrary* m_imageLibrary;
};