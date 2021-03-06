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

#ifndef CHEMKIT_PARTIALCHARGEPREDICTOR_H
#define CHEMKIT_PARTIALCHARGEPREDICTOR_H

#include "chemkit.h"

#include <string>
#include <vector>

namespace chemkit {

class Atom;
class Molecule;
class PartialChargePredictorPrivate;

class CHEMKIT_EXPORT PartialChargePredictor
{
    public:
        // typedefs
        typedef PartialChargePredictor* (*CreateFunction)();

        // construction and destruction
        virtual ~PartialChargePredictor();

        // properties
        std::string name() const;
        void setMolecule(const Molecule *molecule);
        const Molecule* molecule() const;

        // partial charges
        virtual Float partialCharge(int index) const;
        virtual Float partialCharge(const Atom *atom) const;

        // static methods
        static PartialChargePredictor* create(const std::string &name);
        static std::vector<std::string> predictors();
        static bool predictPartialCharges(Molecule *molecule, const std::string &predictorName);

    protected:
        PartialChargePredictor(const std::string &name);
        virtual void assignPartialCharges(const Molecule *molecule);

    private:
        PartialChargePredictorPrivate* const d;
};

} // end chemkit namespace

#endif // CHEMKIT_PARTIALCHARGEPREDICTOR_H
