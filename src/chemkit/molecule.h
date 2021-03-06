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

#ifndef CHEMKIT_MOLECULE_H
#define CHEMKIT_MOLECULE_H

#include "chemkit.h"

#include <string>
#include <vector>

#include <QVariant>

#include "atom.h"
#include "bond.h"
#include "ring.h"
#include "moiety.h"
#include "point3.h"
#include "vector3.h"
#include "fragment.h"
#include "conformer.h"
#include "atommapping.h"

namespace chemkit {

class Coordinates;
class MoleculePrivate;
class MoleculeWatcher;
class InternalCoordinates;

class CHEMKIT_EXPORT Molecule
{
    public:
        // enumerations
        enum ChangeType {
            AtomAdded,
            AtomRemoved,
            AtomAtomicNumberChanged,
            AtomMassNumberChanged,
            AtomPartialChargeChanged,
            AtomPositionChanged,
            AtomChiralityChanged,
            AtomResidueChanged,
            BondAdded,
            BondRemoved,
            BondOrderChanged,
            ConformerAdded,
            ConformerRemoved,
            ConformerChanged,
            NameChanged
        };

        enum CompareFlag {
            CompareAtomsOnly = 0x00,
            CompareHydrogens = 0x01,
            CompareAromaticity = 0x02
        };
        Q_DECLARE_FLAGS(CompareFlags, CompareFlag)

        // construction and destruction
        Molecule();
        Molecule(const std::string &formula, const std::string &format);
        Molecule(const Molecule &molecule);
        ~Molecule();

        // properties
        void setName(const std::string &name);
        std::string name() const;
        std::string formula() const;
        std::string formula(const std::string &format) const;
        QVariant descriptor(const std::string &name) const;
        int size() const;
        bool isEmpty() const;
        Float mass() const;
        void setData(const std::string &name, const QVariant &value);
        QVariant data(const std::string &name) const;

        // structure
        Atom* addAtom(const Element &element);
        Atom* addAtomCopy(const Atom *atom);
        void removeAtom(Atom *atom);
        Atom* atom(int index) const;
        std::vector<Atom *> atoms() const;
        int atomCount() const;
        int atomCount(const Element &element) const;
        bool contains(const Atom *atom) const;
        bool contains(const Element &element) const;
        Bond* addBond(Atom *a, Atom *b, int order = Bond::Single);
        Bond* addBond(int a, int b, int order = Bond::Single);
        void removeBond(Bond *bond);
        void removeBond(Atom *a, Atom *b);
        void removeBond(int a, int b);
        Bond* bond(int index) const;
        Bond* bond(const Atom *a, const Atom *b) const;
        Bond* bond(int a, int b) const;
        std::vector<Bond *> bonds() const;
        int bondCount() const;
        bool contains(const Bond *bond) const;
        void clear();

        // comparison
        bool equals(const Molecule *molecule, CompareFlags flags = CompareFlags()) const;
        bool contains(const Molecule *molecule, CompareFlags flags = CompareFlags()) const;
        bool isSubstructureOf(const Molecule *molecule, CompareFlags flags = CompareFlags()) const;
        AtomMapping mapping(const Molecule *molecule, CompareFlags flags = CompareFlags()) const;
        Moiety find(const Molecule *moiety, CompareFlags flags = CompareFlags()) const;

        // ring perception
        Ring* ring(int index) const;
        std::vector<Ring *> rings() const;
        int ringCount() const;

        // fragment perception
        Fragment* fragment(int index) const;
        std::vector<Fragment *> fragments() const;
        int fragmentCount() const;
        bool isFragmented() const;
        void removeFragment(Fragment *fragment);

        // geometry
        void setCoordinates(const Coordinates *coordinates);
        void setCoordinates(const InternalCoordinates *coordinates);
        Float distance(const Atom *a, const Atom *b) const;
        Float bondAngle(const Atom *a, const Atom *b, const Atom *c) const;
        Float torsionAngle(const Atom *a, const Atom *b, const Atom *c, const Atom *d) const;
        Float wilsonAngle(const Atom *a, const Atom *b, const Atom *c, const Atom *d) const;
        void setCenter(const Point3 &position);
        void setCenter(Float x, Float y, Float z);
        Point3 center() const;
        Point3 centerOfMass() const;
        void moveBy(const Vector3 &vector);
        void moveBy(Float dx, Float dy, Float dz);
        void rotate(const Vector3 &axis, Float angle);
        bool hasCoordinates() const;
        void clearCoordinates();

        // conformers
        Conformer* addConformer();
        void removeConformer(Conformer *conformer);
        void setConformer(Conformer *conformer);
        Conformer* conformer() const;
        Conformer* conformer(int index) const;
        std::vector<Conformer *> conformers() const;
        int conformerCount() const;

        // operators
        Molecule& operator=(const Molecule &molecule);

    private:
        // internal methods
        QList<Atom *> atomPathBetween(const Atom *a, const Atom *b) const;
        int atomCountBetween(const Atom *a, const Atom *b) const;
        int atomCountBetween(const Atom *a, const Atom *b, int maxCount) const;
        QList<Bond *> bondPathBetween(const Atom *a, const Atom *b) const;
        int bondCountBetween(const Atom *a, const Atom *b) const;
        int bondCountBetween(const Atom *a, const Atom *b, int maxCount) const;
        void setRingsPerceived(bool perceived) const;
        bool ringsPerceived() const;
        void setFragmentsPerceived(bool perceived) const;
        bool fragmentsPerceived() const;
        Fragment* fragment(const Atom *atom) const;
        void notifyObservers(ChangeType type);
        void notifyObservers(const Atom *atom, ChangeType type);
        void notifyObservers(const Bond *bond, ChangeType type);
        void notifyObservers(const Conformer *conformer, ChangeType type);
        void addWatcher(MoleculeWatcher *watcher) const;
        void removeWatcher(MoleculeWatcher *watcher) const;
        bool isSubsetOf(const Molecule *molecule, CompareFlags flags = CompareFlags()) const;

        friend class Atom;
        friend class Bond;
        friend class MoleculeWatcher;

    private:
        MoleculePrivate* const d;
        std::vector<Atom *> m_atoms;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Molecule::CompareFlags)

} // end chemkit namespace

#include "molecule-inline.h"

#endif // CHEMKIT_MOLECULE_H
