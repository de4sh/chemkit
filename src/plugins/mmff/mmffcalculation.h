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

#ifndef MMFFCALCULATION_H
#define MMFFCALCULATION_H

#include <chemkit/forcefieldcalculation.h>

class MmffAtom;
class MmffParameters;

class MmffCalculation : public chemkit::ForceFieldCalculation
{
    public:
        const MmffAtom* atom(int index) const;
        virtual bool setup(const MmffParameters *parameters) = 0;

    protected:
        MmffCalculation(int type, int atomCount, int parameterCount);
};

class MmffBondStrechCalculation : public MmffCalculation
{
    public:
        MmffBondStrechCalculation(const MmffAtom *a, const MmffAtom *b);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

class MmffAngleBendCalculation : public MmffCalculation
{
    public:
        MmffAngleBendCalculation(const MmffAtom *a, const MmffAtom *b, const MmffAtom *c);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

class MmffStrechBendCalculation : public MmffCalculation
{
    public:
        MmffStrechBendCalculation(const MmffAtom *a, const MmffAtom *b, const MmffAtom *c);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

class MmffOutOfPlaneBendingCalculation : public MmffCalculation
{
    public:
        MmffOutOfPlaneBendingCalculation(const MmffAtom *a, const MmffAtom *b, const MmffAtom *c, const MmffAtom *d);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

class MmffTorsionCalculation : public MmffCalculation
{
    public:
        MmffTorsionCalculation(const MmffAtom *a, const MmffAtom *b, const MmffAtom *c, const MmffAtom *d);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

class MmffVanDerWaalsCalculation : public MmffCalculation
{
    public:
        MmffVanDerWaalsCalculation(const MmffAtom *a, const MmffAtom *b);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

class MmffElectrostaticCalculation : public MmffCalculation
{
    public:
        MmffElectrostaticCalculation(const MmffAtom *a, const MmffAtom *b);

        bool setup(const MmffParameters *parameters);
        chemkit::Float energy() const;
        std::vector<chemkit::Vector3> gradient() const;
};

#endif // MMFFCALCULATION_H
