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

#include "pubchemtest.h"

#include <chemkit/pubchem.h>
#include <chemkit/molecule.h>
#include <chemkit/moleculefile.h>

void PubChemTest::downloadFile()
{
    chemkit::PubChem pubchem;

    // CID 5950 is alanine
    chemkit::MoleculeFile *file = pubchem.downloadFile("5950");
    QVERIFY(file != 0);

    QCOMPARE(file->moleculeCount(), 1);
    chemkit::Molecule *molecule = file->molecule();
    QCOMPARE(molecule->formula(), std::string("C3H7NO2"));

    delete file;
}

void PubChemTest::downloadMultiFile()
{
    chemkit::PubChem pubchem;

    QStringList ids;
    ids << "1" << "4" << "92" << "8" << "109" << "12";

    chemkit::MoleculeFile *file = pubchem.downloadFile(ids);
    QVERIFY(file != 0);

    QCOMPARE(file->moleculeCount(), 6);

    for(int i = 0; i < ids.size(); i++){
        chemkit::Molecule *molecule = file->molecule(i);
        QVERIFY(molecule != 0);
        QCOMPARE(molecule->name(), ids[i].toStdString());
    }

    delete file;
}

void PubChemTest::search()
{
    chemkit::PubChem pubchem;

    // search for caffeine from is CAS number
    QStringList results = pubchem.search("58-08-2");
    QCOMPARE(results.size(), 1);
    QCOMPARE(results[0], QString("2519"));
}

void PubChemTest::standardizeFormula()
{
    chemkit::PubChem pubchem;

    std::string formula = pubchem.standardizeFormula("c3cccOc3", "smiles");
    QCOMPARE(formula, std::string("C1C=CC=CO1"));
}

QTEST_MAIN(PubChemTest)
