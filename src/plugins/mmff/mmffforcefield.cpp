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

// These files implement the MMFF force field.
//
// Some useful references:
//     Description of MMFF in Towhee:
//          http://towhee.sourceforge.net/forcefields/mmff94.html
//     Parameter Description from CHARMM:
//          http://www.charmm.org/documentation/c32b2/mmff_params.html
//     MMFF Validation Suite:
//          http://server.ccl.net/cca/data/MMFF94/
//     Parameter Data Files:
//          ftp://ftp.wiley.com/public/journals/jcc/suppmat/17/490/MMFF-I_AppendixB.ascii

#include "mmffforcefield.h"

#include "mmffatom.h"
#include "mmffatomtyper.h"
#include "mmffparameters.h"
#include "mmffcalculation.h"
#include "mmffpartialchargepredictor.h"

#include <chemkit/atom.h>
#include <chemkit/bond.h>
#include <chemkit/ring.h>
#include <chemkit/plugin.h>
#include <chemkit/molecule.h>
#include <chemkit/pluginmanager.h>
#include <chemkit/forcefieldinteractions.h>

// --- Construction and Destruction ---------------------------------------- //
MmffForceField::MmffForceField()
    : chemkit::ForceField("mmff"),
      m_parameters(0)
{
    const chemkit::Plugin *mmffPlugin = chemkit::PluginManager::instance()->plugin("mmff");
    if(mmffPlugin){
        std::string dataPath = mmffPlugin->dataPath();
        addParameterSet("mmff94", dataPath + "mmff94.prm");
        setParameterSet("mmff94");
    }

    setFlags(chemkit::ForceField::AnalyticalGradient);
}

MmffForceField::~MmffForceField()
{
    delete m_parameters;
}

// --- Atoms --------------------------------------------------------------- //
MmffAtom* MmffForceField::atom(const chemkit::Atom *atom)
{
    foreach(chemkit::ForceFieldAtom *forceFieldAtom, atoms()){
        if(forceFieldAtom->atom() == atom){
            return static_cast<MmffAtom *>(forceFieldAtom);
        }
    }

    return 0;
}

const MmffAtom* MmffForceField::atom(const chemkit::Atom *atom) const
{
    return const_cast<MmffForceField *>(this)->atom(atom);
}

// --- Parameterization ---------------------------------------------------- //
bool MmffForceField::setup()
{
    if(!m_parameters || m_parameters->fileName() != parameterFile()){
        m_parameters = new MmffParameters;
        bool ok = m_parameters->read(parameterFile());
        if(!ok){
            setErrorString(QString("Failed to load parameters: %1").arg(m_parameters->errorString()).toStdString());
            delete m_parameters;
            m_parameters = 0;
            return false;
        }
    }

    foreach(const chemkit::Molecule *molecule, molecules()){
        MmffAtomTyper typer(molecule);

        // add atoms
        foreach(const chemkit::Atom *atom, molecule->atoms()){
            MmffAtom *mmffAtom = new MmffAtom(this, atom);
            addAtom(mmffAtom);
            mmffAtom->setType(typer.typeNumber(atom), typer.formalCharge(atom));
        }

        // setup atom charges
        MmffPartialChargePredictor partialCharges;
        partialCharges.setAtomTyper(&typer);
        partialCharges.setMolecule(molecule);

        foreach(chemkit::ForceFieldAtom *atom, atoms()){
            atom->setCharge(partialCharges.partialCharge(atom->atom()));
        }

        // add calculations
        chemkit::ForceFieldInteractions interactions(molecule, this);

        // bond strech calculations
        std::pair<const chemkit::ForceFieldAtom *, const chemkit::ForceFieldAtom *> bondedPair;
        foreach(bondedPair, interactions.bondedPairs()){
            const MmffAtom *a = static_cast<const MmffAtom *>(bondedPair.first);
            const MmffAtom *b = static_cast<const MmffAtom *>(bondedPair.second);

            addCalculation(new MmffBondStrechCalculation(a, b));
        }

        // angle bend and strech bend calculations
        std::vector<const chemkit::ForceFieldAtom *> angleGroup;
        foreach(angleGroup, interactions.angleGroups()){
            const MmffAtom *a = static_cast<const MmffAtom *>(angleGroup[0]);
            const MmffAtom *b = static_cast<const MmffAtom *>(angleGroup[1]);
            const MmffAtom *c = static_cast<const MmffAtom *>(angleGroup[2]);

            addCalculation(new MmffAngleBendCalculation(a, b, c));
            addCalculation(new MmffStrechBendCalculation(a, b, c));
        }

        // out of plane bending calculation (for each trigonal center)
        foreach(const chemkit::Atom *atom, molecule->atoms()){
            if(atom->neighborCount() == 3){
                const std::vector<chemkit::Atom *> &neighbors = atom->neighbors();
                const MmffAtom *a = this->atom(neighbors[0]);
                const MmffAtom *b = this->atom(atom);
                const MmffAtom *c = this->atom(neighbors[1]);
                const MmffAtom *d = this->atom(neighbors[2]);

                addCalculation(new MmffOutOfPlaneBendingCalculation(a, b, c, d));
                addCalculation(new MmffOutOfPlaneBendingCalculation(a, b, d, c));
                addCalculation(new MmffOutOfPlaneBendingCalculation(c, b, d, a));
            }
        }

        // torsion calculations (for each dihedral)
        std::vector<const chemkit::ForceFieldAtom *> torsionGroup;
        foreach(torsionGroup, interactions.torsionGroups()){
            const MmffAtom *a = static_cast<const MmffAtom *>(torsionGroup[0]);
            const MmffAtom *b = static_cast<const MmffAtom *>(torsionGroup[1]);
            const MmffAtom *c = static_cast<const MmffAtom *>(torsionGroup[2]);
            const MmffAtom *d = static_cast<const MmffAtom *>(torsionGroup[3]);

            addCalculation(new MmffTorsionCalculation(a, b, c, d));
        }

        // van der waals and electrostatic calculations
        std::pair<const chemkit::ForceFieldAtom *, const chemkit::ForceFieldAtom *> nonbondedPair;
        foreach(nonbondedPair, interactions.nonbondedPairs()){
            const MmffAtom *a = static_cast<const MmffAtom *>(nonbondedPair.first);
            const MmffAtom *b = static_cast<const MmffAtom *>(nonbondedPair.second);

            addCalculation(new MmffVanDerWaalsCalculation(a, b));
            addCalculation(new MmffElectrostaticCalculation(a, b));
        }
    }

    bool ok = true;

    foreach(chemkit::ForceFieldCalculation *calculation, calculations()){
        bool setup = static_cast<MmffCalculation *>(calculation)->setup(m_parameters);

        if(!setup){
            ok = false;
        }

        setCalculationSetup(calculation, setup);
    }

    return ok;
}

const MmffParameters* MmffForceField::parameters() const
{
    return m_parameters;
}

// --- Static Methods ------------------------------------------------------ //
bool MmffForceField::isAromatic(const chemkit::Ring *ring)
{
    if(ring->size() != 5 && ring->size() != 6)
        return false;

    int piCount = piElectronCount(ring);

    // count exocyclic aromatic bonds
    foreach(const chemkit::Atom *atom, ring->atoms()){
        foreach(const chemkit::Bond *bond, atom->bonds()){
            if(ring->contains(bond))
                continue;

            if(bond->order() == chemkit::Bond::Double){
                foreach(const chemkit::Ring *otherRing, bond->rings()){
                    if(otherRing == ring)
                        continue;

                    else if(piElectronCount(otherRing) == 6)
                        piCount += 1;
                }
            }
        }
    }

    return piCount == 6;
}

bool MmffForceField::isAromatic(const chemkit::Atom *atom)
{
    foreach(const chemkit::Ring *ring, atom->rings()){
        if(isAromatic(ring)){
            return true;
        }
    }

    return false;
}

bool MmffForceField::isAromatic(const chemkit::Bond *bond)
{
    foreach(const chemkit::Ring *ring, bond->rings()){
        if(isAromatic(ring)){
            return true;
        }
    }

    return false;
}

int MmffForceField::piElectronCount(const chemkit::Ring *ring)
{
    int piElectronCount = 0;

    // ring lone pair donors
    foreach(const chemkit::Atom *atom, ring->atoms()){
        if(ring->size() == 5){
            if(atom->is(chemkit::Atom::Nitrogen) &&
               atom->neighborCount() == 3 &&
               atom->valence() == 3){
                piElectronCount += 2;
                break;
            }
            else if(atom->is(chemkit::Atom::Nitrogen) &&
                    atom->neighborCount() == 2 &&
                    atom->valence() == 2){
                piElectronCount += 2;
                break;
            }
            else if((atom->is(chemkit::Atom::Oxygen) || atom->is(chemkit::Atom::Sulfur)) &&
                    atom->neighborCount() == 2){
                piElectronCount += 2;
                break;
            }
        }
    }

    // ring double bonds
    foreach(const chemkit::Bond *bond, ring->bonds()){
        if(bond->order() == chemkit::Bond::Double){
            piElectronCount += 2;
        }
    }

    return piElectronCount;
}
