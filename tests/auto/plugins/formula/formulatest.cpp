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

#include "formulatest.h"

#include <algorithm>

#include <chemkit/molecule.h>
#include <chemkit/lineformat.h>

void FormulaTest::initTestCase()
{
    std::vector<std::string> formats = chemkit::LineFormat::formats();
    QVERIFY(std::find(formats.begin(), formats.end(), "formula") != formats.end());
}

void FormulaTest::read()
{
    chemkit::LineFormat *formulaFormat = chemkit::LineFormat::create("formula");
    QVERIFY(formulaFormat != 0);

    // empty
    chemkit::Molecule *empty = formulaFormat->read("");
    QCOMPARE(empty->isEmpty(), true);
    delete empty;

    // hexane
    chemkit::Molecule *hexane = formulaFormat->read("C6H14");
    QCOMPARE(hexane->atomCount(), 20);
    QCOMPARE(hexane->atomCount(chemkit::Atom::Carbon), 6);
    QCOMPARE(hexane->atomCount(chemkit::Atom::Hydrogen), 14);
    delete hexane;

    // water
    chemkit::Molecule *water = formulaFormat->read("H2O");
    QCOMPARE(water->atomCount(), 3);
    QCOMPARE(water->atomCount(chemkit::Atom::Hydrogen), 2);
    QCOMPARE(water->atomCount(chemkit::Atom::Oxygen), 1);
    delete water;

    // atp
    chemkit::Molecule *atp = formulaFormat->read("C10H16N5O13P3");
    QCOMPARE(atp->atomCount(), 47);
    QCOMPARE(atp->atomCount(chemkit::Atom::Carbon), 10);
    QCOMPARE(atp->atomCount(chemkit::Atom::Hydrogen), 16);
    QCOMPARE(atp->atomCount(chemkit::Atom::Nitrogen), 5);
    QCOMPARE(atp->atomCount(chemkit::Atom::Oxygen), 13);
    QCOMPARE(atp->atomCount(chemkit::Atom::Phosphorus), 3);
    delete atp;

    delete formulaFormat;
}

void FormulaTest::write()
{
    chemkit::LineFormat *formulaFormat = chemkit::LineFormat::create("formula");
    QVERIFY(formulaFormat != 0);

    // empty
    chemkit::Molecule empty;
    QCOMPARE(formulaFormat->write(&empty), std::string());

    // water
    chemkit::Molecule water;
    water.addAtom("H");
    water.addAtom("H");
    water.addAtom("O");
    QCOMPARE(formulaFormat->write(&water), std::string("H2O"));

    delete formulaFormat;
}

QTEST_APPLESS_MAIN(FormulaTest)
