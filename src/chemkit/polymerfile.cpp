/******************************************************************************
**
** Copyright (C) 2009-2011 Kyle Lutz <kyle.r.lutz@gmail.com>
** All rights reserved.
**
** This file is a part of the chemkit project. For more information
** see <http://www.chemkit.org>.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in the
**     documentation and/or other materials provided with the distribution.
**   * Neither the name of the chemkit project nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
******************************************************************************/

#include "polymerfile.h"

#include "polymer.h"
#include "polymerfileformat.h"

namespace chemkit {

// === PolymerFilePrivate ================================================== //
class PolymerFilePrivate
{
    public:
        std::string fileName;
        std::string errorString;
        PolymerFileFormat *format;
        std::vector<Polymer *> polymers;
};

// === PolymerFile ========================================================= //
/// \class PolymerFile polymerfile.h chemkit/polymerfile.h
/// \ingroup chemkit
/// \brief The PolymerFile class contains polymers.
///
/// \see Polymer

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new polymer file.
PolymerFile::PolymerFile()
    : d(new PolymerFilePrivate)
{
    d->format = 0;
}

/// Creates a new polymer file with \p fileName.
PolymerFile::PolymerFile(const std::string &fileName)
    : d(new PolymerFilePrivate)
{
    d->format = 0;
    d->fileName = fileName;
}

/// Destroys the polymer file object.
PolymerFile::~PolymerFile()
{
    qDeleteAll(d->polymers);
    delete d->format;
    delete d;
}

// --- Properties ---------------------------------------------------------- //
/// Sets the file name for the file to \p fileName.
void PolymerFile::setFileName(const std::string &fileName)
{
    d->fileName = fileName;
}

/// Returns the file name for the file.
std::string PolymerFile::fileName() const
{
    return d->fileName;
}

/// Sets the format for the file to \p format.
void PolymerFile::setFormat(PolymerFileFormat *format)
{
    d->format = format;
}

/// Sets the format for the file to \p name. Returns \c false if an
/// error occured.
bool PolymerFile::setFormat(const std::string &name)
{
    PolymerFileFormat *format = PolymerFileFormat::create(name);
    if(!format){
        return false;
    }

    setFormat(format);
    return true;
}

/// Returns the file format for the file.
PolymerFileFormat* PolymerFile::format() const
{
    return d->format;
}

/// Returns the name of the file format for the file.
std::string PolymerFile::formatName() const
{
    if(d->format){
        return d->format->name();
    }

    return std::string();
}

/// Returns the number of polymers in the file.
int PolymerFile::size() const
{
    return polymerCount();
}

/// Returns \c true if the file contains no polymers.
bool PolymerFile::isEmpty() const
{
    return size() == 0;
}

// --- File Contents ------------------------------------------------------- //
/// Adds a polymer to the file.
///
/// The ownership of the polymer is passed to the file.
void PolymerFile::addPolymer(Polymer *polymer)
{
    d->polymers.push_back(polymer);
}

/// Removes a polymer from the file.
///
/// The ownership of the polymer is passed to the caller.
bool PolymerFile::removePolymer(Polymer *polymer)
{
    std::vector<Polymer *>::iterator location = std::find(d->polymers.begin(), d->polymers.end(), polymer);
    if(location == d->polymers.end()){
        return false;
    }

    d->polymers.erase(location);

    return true;
}

/// Removes a polymer from the file and deletes it.
bool PolymerFile::deletePolymer(Polymer *polymer)
{
    bool found = removePolymer(polymer);

    if(found){
        delete polymer;
    }

    return found;
}

/// Returns the polymer at \p index in the file.
Polymer* PolymerFile::polymer(int index) const
{
    return d->polymers[index];
}

/// Returns a list of all the polymers in the file.
std::vector<Polymer *> PolymerFile::polymers() const
{
    return d->polymers;
}

/// Returns the number of polymers in the file.
int PolymerFile::polymerCount() const
{
    return d->polymers.size();
}

/// Returns \c true if the file contains \p polymer.
bool PolymerFile::contains(const Polymer *polymer) const
{
    return std::find(d->polymers.begin(), d->polymers.end(), polymer) != d->polymers.end();
}

/// Removes all the polymers from the file.
void PolymerFile::clear()
{
    qDeleteAll(d->polymers);
    d->polymers.clear();
}

// --- Input and Output ---------------------------------------------------- //
/// Reads the file.
bool PolymerFile::read()
{
    if(d->fileName.empty()){
        return false;
    }

    return read(fileName());
}

/// Reads the file from \p fileName.
bool PolymerFile::read(const std::string &fileName)
{
    std::string format = QFileInfo(fileName.c_str()).suffix().toStdString();

    return read(fileName, format);
}

/// Reads the file from \p fileName using format.
bool PolymerFile::read(const std::string &fileName, const std::string &format)
{
    QFile file(fileName.c_str());
    if(!file.open(QIODevice::ReadOnly)){
        setErrorString(QString("Failed to open '%1' for reading: %2").arg(fileName.c_str()).arg(file.errorString()).toStdString());
        return false;
    }

    return read(&file, format);
}

/// Reads the file from \p iodev using \p format.
bool PolymerFile::read(QIODevice *iodev, const std::string &format)
{
    if(d->format == 0 || d->format->name() != format){
        d->format = PolymerFileFormat::create(format);
        if(!d->format){
            setErrorString(QString("Format '%1' is not supported").arg(format.c_str()).toStdString());
            iodev->close();
            return false;
        }
    }

    bool ok = d->format->read(iodev, this);
    if(!ok){
        setErrorString(d->format->errorString().c_str());
    }

    iodev->close();
    return ok;
}

/// Writes the file.
bool PolymerFile::write()
{
    return write(fileName());
}

/// Writes the file to \p fileName.
bool PolymerFile::write(const std::string &fileName)
{
    std::string format = QFileInfo(fileName.c_str()).suffix().toStdString();

    return write(fileName, format);
}

/// Writes the file to \p fileName using \p format.
bool PolymerFile::write(const std::string &fileName, const std::string &format)
{
    QFile file(fileName.c_str());
    if(!file.open(QIODevice::WriteOnly)){
        setErrorString(QString("Failed to open '%1' for writing: %2").arg(fileName.c_str()).arg(file.errorString()).toStdString());
        return false;
    }

    return write(&file, format);
}

/// Writes the file to \p iodev.
bool PolymerFile::write(QIODevice *iodev)
{
    if(!d->format)
        return false;

    bool ok = d->format->write(this, iodev);
    if(!ok){
        setErrorString(d->format->errorString().c_str());
    }

    iodev->close();
    return ok;
}

/// Writes the file to \p iodev using \p format.
bool PolymerFile::write(QIODevice *iodev, const std::string &format)
{
    if(!d->format || d->format->name() != format){
        d->format = PolymerFileFormat::create(format);
        if(!d->format){
            setErrorString(QString("Format '%1' is not supported").arg(format.c_str()).toStdString());
            iodev->close();
            return false;
        }
    }

    return write(iodev);
}

// --- Error Handling ------------------------------------------------------ //
void PolymerFile::setErrorString(const std::string &errorString)
{
    d->errorString = errorString;
}

/// Returns a string describing the last error that occured.
std::string PolymerFile::errorString() const
{
    return d->errorString;
}

// --- Static Methods ------------------------------------------------------ //
/// Returns a list of all supported polymer file formats.
std::vector<std::string> PolymerFile::formats()
{
    return PolymerFileFormat::formats();
}

} // end chemkit namespace
