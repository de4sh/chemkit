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

#ifndef CHEMKIT_MOLECULEWATCHER_H
#define CHEMKIT_MOLECULEWATCHER_H

#include "chemkit.h"

#include "molecule.h"

namespace chemkit {

class Atom;
class Bond;
class Molecule;
class Conformer;
class MoleculeWatcherPrivate;

class CHEMKIT_EXPORT MoleculeWatcher : public QObject
{
    Q_OBJECT

    public:
        // construction and destruction
        MoleculeWatcher(const Molecule *molecule = 0);
        ~MoleculeWatcher();

        // properties
        void setMolecule(const Molecule *molecule);
        const Molecule* molecule() const;

    Q_SIGNALS:
        void atomAdded(const chemkit::Atom *atom);
        void atomRemoved(const chemkit::Atom *atom);
        void atomAtomicNumberChanged(const chemkit::Atom *atom);
        void atomPositionChanged(const chemkit::Atom *atom);
        void bondAdded(const chemkit::Bond *bond);
        void bondRemoved(const chemkit::Bond *bond);
        void bondOrderChanged(const chemkit::Bond *bond);
        void conformerAdded(const chemkit::Conformer *conformer);
        void conformerRemoved(const chemkit::Conformer *conformer);
        void nameChanged(const chemkit::Molecule *molecule);

    private:
        void notifyObservers(const Molecule *molecule, Molecule::ChangeType changeType);
        void notifyObservers(const Atom *atom, Molecule::ChangeType changeType);
        void notifyObservers(const Bond *bond, Molecule::ChangeType changeType);
        void notifyObservers(const Conformer *conformer, Molecule::ChangeType changeType);

        friend class Molecule;

    private:
        MoleculeWatcherPrivate* const d;
};

} // end chemkit namespace

#endif // CHEMKIT_MOLECULEWATCHER_H
