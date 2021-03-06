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

#include "fragment.h"

#include <cassert>

#include "foreach.h"
#include "molecule.h"

namespace chemkit {

// === Fragment ============================================================ //
/// \class Fragment fragment.h chemkit/fragment.h
/// \ingroup chemkit
/// \brief The Fragment class represents a group of connected atoms in
///        a molecule.
///
/// Fragment objects are returned from the various fragment perception
/// methods such as Molecule::fragments() and Atom::fragment().

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new fragment that contains all the atoms attached to
/// \p root.
Fragment::Fragment(Atom *root)
{
    assert(root->m_fragment == 0);

    std::vector<Atom *> row;
    row.push_back(root);

    while(!row.empty()){
        std::vector<Atom *> nextRow;

        foreach(Atom *atom, row){
            if(!atom->m_fragment){
                atom->m_fragment = this;
                m_atoms.push_back(atom);

                foreach(Atom *neighbor, atom->neighbors()){
                    nextRow.push_back(neighbor);
                }
            }
        }

        row = nextRow;
    }
}

/// Destroys the fragment object.
Fragment::~Fragment()
{
}

// --- Structure ----------------------------------------------------------- //
/// Returns a list of all the bonds in the fragment.
std::vector<Bond *> Fragment::bonds() const
{
    std::vector<Bond *> bonds;

    foreach(Atom *atom, m_atoms){
        foreach(Bond *bond, atom->bonds()){
            if(std::find(bonds.begin(), bonds.end(), bond) == bonds.end()){
                bonds.push_back(bond);
            }
        }
    }

    return bonds;
}

/// Returns the number of bonds in the fragment.
int Fragment::bondCount() const
{
    return bonds().size();
}

/// Returns \c true if the fragment contains the bond.
bool Fragment::contains(const Bond *bond) const
{
    return bond->atom1()->fragment() == this;
}

} // end chemkit namespace
