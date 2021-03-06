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

#ifndef AMBERPARAMETERS_H
#define AMBERPARAMETERS_H

#include <chemkit/forcefieldatom.h>

struct AmberBondParameters
{
    chemkit::Float kb;
    chemkit::Float r0;
};

struct AmberAngleParameters
{
    chemkit::Float ka;
    chemkit::Float theta0;
};

struct AmberTorsionParameters
{
    chemkit::Float V1;
    chemkit::Float V2;
    chemkit::Float V3;
    chemkit::Float V4;
    chemkit::Float gamma1;
    chemkit::Float gamma2;
    chemkit::Float gamma3;
    chemkit::Float gamma4;
};

struct AmberNonbondedParameters
{
    chemkit::Float vanDerWaalsRadius;
    chemkit::Float wellDepth;
};

class AmberParameters
{
    public:
        // construction and destruction
        AmberParameters();
        ~AmberParameters();

        // parameters
        const AmberBondParameters* bondParameters(const chemkit::ForceFieldAtom *a, const chemkit::ForceFieldAtom *b) const;
        const AmberAngleParameters* angleParameters(const chemkit::ForceFieldAtom *a, const chemkit::ForceFieldAtom *b, const chemkit::ForceFieldAtom *c) const;
        const AmberTorsionParameters* torsionParameters(const chemkit::ForceFieldAtom *a, const chemkit::ForceFieldAtom *b, const chemkit::ForceFieldAtom *c, const chemkit::ForceFieldAtom *d) const;
        const AmberNonbondedParameters* nonbondedParameters(const chemkit::ForceFieldAtom *atom) const;
};

#endif // AMBERPARAMETERS_H
