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

#include "moleculealignertest.h"

#include <chemkit/polymer.h>
#include <chemkit/molecule.h>
#include <chemkit/polymerfile.h>
#include <chemkit/polymerchain.h>
#include <chemkit/moleculealigner.h>

const std::string dataPath = "../../../data/";

#define COMPARE_DOUBLES(actual, expected) QVERIFY(qAbs(actual - expected) < 0.001)

void MoleculeAlignerTest::water()
{
    chemkit::Molecule water1;
    chemkit::Atom *O1 = water1.addAtom("O");
    chemkit::Atom *H2 = water1.addAtom("H");
    chemkit::Atom *H3 = water1.addAtom("H");
    O1->setPosition(0, 0, 0);
    H2->setPosition(1, 0, 0);
    H3->setPosition(0, 1, 0);

    chemkit::Molecule water2;
    chemkit::Atom *O4 = water2.addAtom("O");
    chemkit::Atom *H5 = water2.addAtom("H");
    chemkit::Atom *H6 = water2.addAtom("H");
    O4->setPosition(0, 0, 0);
    H5->setPosition(-1, 0, 0);
    H6->setPosition(0, 1, 0);

    chemkit::MoleculeAligner aligner(&water1, &water2);
    QCOMPARE(aligner.mapping().size(), 3);
    COMPARE_DOUBLES(aligner.deviation(), 1.1547);

    aligner.align(&water1);
    COMPARE_DOUBLES(aligner.deviation(), 0.0);
}

// This test verifies the alignment algorithm using a pdb file containing
// 10 conformers. For each the RMSD is compared against the first conformer
// and then each is aligned with the first conformer and the minimized RMSD
// is checked.
//
// The expected RMSD values were calculated using pymol's intra_rms command.
// After loading the 1D3Z.pdb file use: "print cmd.intra_rms_cur('1D3Z')" to
// to obtain the initial RMSD values. Next use: "print cmd.intra_rms('1D3Z')"
// to perform the alignment and obtain the minimized RMSD values.
void MoleculeAlignerTest::ubiquitin()
{
    chemkit::PolymerFile file(dataPath + "1D3Z.pdb");
    bool ok = file.read();
    if(!ok)
        qDebug() << file.errorString().c_str();
    QVERIFY(ok);

    QCOMPARE(file.polymerCount(), 1);
    chemkit::Polymer *polymer = file.polymer();

    QCOMPARE(polymer->chainCount(), 1);
    chemkit::PolymerChain *chain = polymer->chain(0);
    QCOMPARE(chain->residueCount(), 76);

    chemkit::Molecule *molecule = polymer;
    QCOMPARE(molecule->atomCount(), 1231);
    QCOMPARE(molecule->conformerCount(), 10);

    chemkit::MoleculeAligner aligner(molecule, molecule);
    QCOMPARE(aligner.mapping().size(), 1231);

    // verify rmsd values for each conformer
    aligner.setTargetConformer(molecule->conformers()[1]);
    COMPARE_DOUBLES(aligner.deviation(), 2.29165);
    aligner.setTargetConformer(molecule->conformers()[2]);
    COMPARE_DOUBLES(aligner.deviation(), 1.51009);
    aligner.setTargetConformer(molecule->conformers()[3]);
    COMPARE_DOUBLES(aligner.deviation(), 1.98526);
    aligner.setTargetConformer(molecule->conformers()[4]);
    COMPARE_DOUBLES(aligner.deviation(), 1.87933);
    aligner.setTargetConformer(molecule->conformers()[5]);
    COMPARE_DOUBLES(aligner.deviation(), 2.27420);
    aligner.setTargetConformer(molecule->conformers()[6]);
    COMPARE_DOUBLES(aligner.deviation(), 2.61271);
    aligner.setTargetConformer(molecule->conformers()[7]);
    COMPARE_DOUBLES(aligner.deviation(), 2.78852);
    aligner.setTargetConformer(molecule->conformers()[8]);
    COMPARE_DOUBLES(aligner.deviation(), 2.59195);
    aligner.setTargetConformer(molecule->conformers()[9]);
    COMPARE_DOUBLES(aligner.deviation(), 2.26074);

    // align molecule to each conformer and verified minimized rmsd values
    aligner.setTargetConformer(molecule->conformers()[1]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.05756);
    aligner.setTargetConformer(molecule->conformers()[2]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.32468);
    aligner.setTargetConformer(molecule->conformers()[3]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.41645);
    aligner.setTargetConformer(molecule->conformers()[4]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.39656);
    aligner.setTargetConformer(molecule->conformers()[5]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.81463);
    aligner.setTargetConformer(molecule->conformers()[6]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.78510);
    aligner.setTargetConformer(molecule->conformers()[7]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 2.04545);
    aligner.setTargetConformer(molecule->conformers()[8]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.39502);
    aligner.setTargetConformer(molecule->conformers()[9]);
    aligner.align(molecule);
    COMPARE_DOUBLES(aligner.deviation(), 1.26402);
}

QTEST_APPLESS_MAIN(MoleculeAlignerTest)
