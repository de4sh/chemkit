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

#include "uridineminimizationbenchmark.h"

#include <chemkit/molecule.h>
#include <chemkit/forcefield.h>
#include <chemkit/moleculefile.h>

const std::string dataPath = "../../data/";

void UridineMinimizationBenchmark::benchmark()
{
    chemkit::Molecule *molecule = chemkit::MoleculeFile::quickRead(dataPath + "uridine.mol2");
    QVERIFY(molecule != 0);

    chemkit::ForceField *forceField = chemkit::ForceField::create("uff");
    QVERIFY(forceField != 0);

    forceField->addMolecule(molecule);
    bool ok = forceField->setup();
    QVERIFY(ok);

    QBENCHMARK {
        for(;;){
            // converge when rmsg = 0.1
            bool converged = forceField->minimizationStep(0.1);

            if(converged){
                break;
            }
        }
    }

    delete forceField;
    delete molecule;
}

QTEST_APPLESS_MAIN(UridineMinimizationBenchmark)
