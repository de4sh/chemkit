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

#include "uffcalculation.h"
#include "uffforcefield.h"
#include "uffparameters.h"

#include <chemkit/atom.h>
#include <chemkit/bond.h>
#include <chemkit/molecule.h>
#include <chemkit/constants.h>

// === UffCalculation ====================================================== //
UffCalculation::UffCalculation(int type, int atomCount, int parameterCount)
    : ForceFieldCalculation(type, atomCount, parameterCount)
{
}

// Returns the parameters for the given atom.
const UffAtomParameters* UffCalculation::parameters(const chemkit::ForceFieldAtom *atom) const
{
    const UffForceField *forceField = static_cast<const UffForceField *>(this->forceField());
    const UffParameters *parameters = forceField->parameters();

    return parameters->parameters(atom);
}

// Returns the bond order of the bond between atom's a and b. If both
// atoms have a resonant type the bond order returned is 1.5.
// Otherwise the integer value of the bond order is returned.
chemkit::Float UffCalculation::bondOrder(const chemkit::ForceFieldAtom *a, const chemkit::ForceFieldAtom *b) const
{
    const chemkit::Bond *bond = a->atom()->bondTo(b->atom());

    if((a->type().length() > 2 && a->type()[2] == 'R') && (b->type().length() > 2 && b->type()[2] == 'R')){
        return 1.5; // resonant
    }
    else{
        return bond->order();
    }
}

// Returns the length of the bond between two atoms.
chemkit::Float UffCalculation::bondLength(const UffAtomParameters *a, const UffAtomParameters *b, chemkit::Float bondOrder) const
{
    // r_ij = r_i + r_j + r_bo - r_en
    chemkit::Float r_bo = -0.1332 * (a->r + b->r) * log(bondOrder);
    chemkit::Float r_en = ((a->r * b->r) * pow((sqrt(a->X) - sqrt(b->X)), 2)) / (a->X*a->r + b->X*b->r);

    chemkit::Float r_ij = a->r + b->r + r_bo - r_en;

    return r_ij;
}

// === UffBondStrechCalculation ============================================ //
UffBondStrechCalculation::UffBondStrechCalculation(const chemkit::ForceFieldAtom *a, const chemkit::ForceFieldAtom *b)
    : UffCalculation(BondStrech, 2, 2)
{
    setAtom(0, a);
    setAtom(1, b);
}

bool UffBondStrechCalculation::setup()
{
    const UffAtomParameters *pa = parameters(atom(0));
    const UffAtomParameters *pb = parameters(atom(1));

    if(!pa || !pb){
        return false;
    }

    // n = bondorder (1.5 for aromatic, 1.366 for amide)
    chemkit::Float bondorder = bondOrder(atom(0), atom(1));

    chemkit::Float r0 = bondLength(pa, pb, bondorder);

    // parameter(1) = k_ij = 664.12 * (Z*_i * Z*_j) / r_ij^3
    chemkit::Float za = pa->Z;
    chemkit::Float zb = pb->Z;
    chemkit::Float kb = 664.12 * (za * zb) / pow(r0, 3);

    setParameter(0, kb);
    setParameter(1, r0);

    return true;
}

chemkit::Float UffBondStrechCalculation::energy() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);

    chemkit::Float kb = parameter(0);
    chemkit::Float r0 = parameter(1);
    chemkit::Float r = distance(a, b);

    return 0.5 * kb * pow(r - r0, 2);
}

std::vector<chemkit::Vector3> UffBondStrechCalculation::gradient() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);

    chemkit::Float kb = parameter(0);
    chemkit::Float r0 = parameter(1);
    chemkit::Float r = distance(a, b);

    // dE/dr
    chemkit::Float de_dr = kb * (r - r0);

    std::vector<chemkit::Vector3> gradient = distanceGradient(a, b);

    gradient[0] *= de_dr;
    gradient[1] *= de_dr;

    return gradient;
}

// === UffAngleBendCalculation ============================================= //
UffAngleBendCalculation::UffAngleBendCalculation(const chemkit::ForceFieldAtom *a,
                                                 const chemkit::ForceFieldAtom *b,
                                                 const chemkit::ForceFieldAtom *c)
    : UffCalculation(AngleBend, 3, 4)
{
    setAtom(0, a);
    setAtom(1, b);
    setAtom(2, c);
}

bool UffAngleBendCalculation::setup()
{
    const UffAtomParameters *pa = parameters(atom(0));
    const UffAtomParameters *pb = parameters(atom(1));
    const UffAtomParameters *pc = parameters(atom(2));

    if(!pa || !pb || !pc){
        return false;
    }

    chemkit::Float theta0 = pb->theta * chemkit::constants::DegreesToRadians;

    const chemkit::Bond *bond_ab = atom(0)->atom()->bondTo(atom(1)->atom());
    const chemkit::Bond *bond_bc = atom(1)->atom()->bondTo(atom(2)->atom());

    chemkit::Float bo_ij = bond_ab->order();
    chemkit::Float bo_jk = bond_bc->order();

    chemkit::Float r_ab = bondLength(pa, pb, bo_ij);
    chemkit::Float r_bc = bondLength(pb, pc, bo_jk);
    chemkit::Float r_ac = sqrt(pow(r_ab, 2)  + pow(r_bc, 2) - (2.0 * r_ab * r_bc * cos(theta0)));

    chemkit::Float beta = 664.12 / (r_ab * r_bc);

    chemkit::Float z_a = pa->Z;
    chemkit::Float z_c = pc->Z;

    // equation 13
    chemkit::Float ka = beta * ((z_a * z_c) / pow(r_ac, 5)) * r_ab * r_bc * (3.0 * r_ab * r_bc * (1.0 - pow(cos(theta0), 2.0)) - (pow(r_ac, 2.0) * cos(theta0)));

    setParameter(0, ka);

    chemkit::Float c2 = 1 / (4 * pow(sin(theta0), 2));
    chemkit::Float c1 = -4 * c2 * cos(theta0);
    chemkit::Float c0 = c2 * (2 * pow(cos(theta0), 2) + 1);

    setParameter(1, c0);
    setParameter(2, c1);
    setParameter(3, c2);

    return true;
}

chemkit::Float UffAngleBendCalculation::energy() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);

    chemkit::Float ka = parameter(0);
    chemkit::Float c0 = parameter(1);
    chemkit::Float c1 = parameter(2);
    chemkit::Float c2 = parameter(3);

    chemkit::Float theta = bondAngleRadians(a, b, c);

    return ka * (c0 + (c1 * cos(theta)) + (c2 * cos(2*theta)));
}

std::vector<chemkit::Vector3> UffAngleBendCalculation::gradient() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);

    chemkit::Float ka = parameter(0);
    chemkit::Float c1 = parameter(2);
    chemkit::Float c2 = parameter(3);

    chemkit::Float theta = bondAngleRadians(a, b, c);

    // dE/dtheta
    chemkit::Float de_dtheta = -ka * (c1 * sin(theta) + 2 * c2 * sin(2 * theta));

    std::vector<chemkit::Vector3> gradient = bondAngleGradientRadians(a, b, c);

    gradient[0] *= de_dtheta;
    gradient[1] *= de_dtheta;
    gradient[2] *= de_dtheta;

    return gradient;
}

// === UffTorsionCalculation =============================================== //
UffTorsionCalculation::UffTorsionCalculation(const chemkit::ForceFieldAtom *a,
                                             const chemkit::ForceFieldAtom *b,
                                             const chemkit::ForceFieldAtom *c,
                                             const chemkit::ForceFieldAtom *d)
    : UffCalculation(Torsion, 4, 3)
{
    setAtom(0, a);
    setAtom(1, b);
    setAtom(2, c);
    setAtom(3, d);
}

bool UffTorsionCalculation::setup()
{
    UffForceField *forceField = static_cast<UffForceField *>(this->forceField());

    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);

    if(b->type().length() < 3 || c->type().length() < 3){
        return false;
    }

    const UffAtomParameters *pb = parameters(b);
    const UffAtomParameters *pc = parameters(c);

    chemkit::Float V = 0;
    chemkit::Float n = 0;
    chemkit::Float phi0 = 0;

    // sp3-sp3
    if(b->type()[2] == '3' && c->type()[2] == '3'){

        // exception for two group six atoms
        if(forceField->isGroupSix(b) && forceField->isGroupSix(c)){
            if(b->atom()->is(chemkit::Atom::Oxygen) && c->atom()->is(chemkit::Atom::Oxygen)){
                V = 2; // sqrt(2*2)
            }
            else if(b->atom()->is(chemkit::Atom::Oxygen) || c->atom()->is(chemkit::Atom::Oxygen)){
                V = sqrt(2 * 6.8);
            }
            else{
                V = sqrt(6.8 * 6.8);
            }

            n = 2;
            phi0 = 90;
        }

        // general case
        else{
            // equation 16
            V = sqrt(pb->V * pc->V);

            n = 3;
            phi0 = 180 * chemkit::constants::DegreesToRadians;
        }
    }
    // sp2-sp2
    else if((b->type()[2] == '2' || b->type()[2] == 'R') && (c->type()[2] == '2' || c->type()[2] == 'R')){
        chemkit::Float bondorder = bondOrder(b, c);

        // equation 17
        V = 5 * sqrt(pb->U * pc->U) * (1 + 4.18 * log(bondorder));

        n = 2;
        phi0 = 180 * chemkit::constants::DegreesToRadians;
    }
    // group 6 sp3 - any sp2 or R
    else if((forceField->isGroupSix(b) && (c->type()[2] == '2' || c->type()[2] == 'R')) ||
            (forceField->isGroupSix(c) && (b->type()[2] == '2' || b->type()[2] == 'R'))){
        chemkit::Float bondorder = bondOrder(b, c);

        // equation 17
        V = 5 * sqrt(pb->U * pc->U) * (1 + 4.18 * log(bondorder));

        n = 2;
        phi0 = 90 * chemkit::constants::DegreesToRadians;
    }
    // sp3-sp2
    else if((b->type()[2] == '3' && (c->type()[2] == '2' || c->type()[2] == 'R')) ||
            (c->type()[2] == '3' && (b->type()[2] == '2' || b->type()[2] == 'R'))){
        V = 1;
        n = 6;
        phi0 = 0;
    }
    else{
        return false;
    }

    setParameter(0, V);
    setParameter(1, n);
    setParameter(2, phi0);

    return true;
}

chemkit::Float UffTorsionCalculation::energy() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);
    const chemkit::ForceFieldAtom *d = atom(3);

    chemkit::Float V = parameter(0);
    chemkit::Float n = parameter(1);
    chemkit::Float phi0 = parameter(2);

    chemkit::Float phi = torsionAngleRadians(a, b, c, d);

    return 0.5 * V * (1 - cos(n * phi0) * cos(n * phi));
}

std::vector<chemkit::Vector3> UffTorsionCalculation::gradient() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);
    const chemkit::ForceFieldAtom *d = atom(3);

    chemkit::Float V = parameter(0);
    chemkit::Float n = parameter(1);
    chemkit::Float phi0 = parameter(2);

    chemkit::Float phi = torsionAngleRadians(a, b, c, d);

    // dE/dphi
    chemkit::Float de_dphi = 0.5 * V * n * cos(n * phi0) * sin(n * phi);

    std::vector<chemkit::Vector3> gradient = torsionAngleGradientRadians(a, b, c, d);

    gradient[0] *= de_dphi;
    gradient[1] *= de_dphi;
    gradient[2] *= de_dphi;
    gradient[3] *= de_dphi;

    return gradient;
}

// === UffInversionCalculation ============================================= //
UffInversionCalculation::UffInversionCalculation(const chemkit::ForceFieldAtom *a,
                                                 const chemkit::ForceFieldAtom *b,
                                                 const chemkit::ForceFieldAtom *c,
                                                 const chemkit::ForceFieldAtom *d)
    : UffCalculation(Inversion, 4, 4)
{
    setAtom(0, a);
    setAtom(1, b);
    setAtom(2, c);
    setAtom(3, d);
}

bool UffInversionCalculation::setup()
{
    // b is the center atom
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);
    const chemkit::ForceFieldAtom *d = atom(3);

    chemkit::Float k = 0;
    chemkit::Float c0 = 0;
    chemkit::Float c1 = 0;
    chemkit::Float c2 = 0;

    // sp2 carbon
    if(b->type() == "C_2" || b->type() == "C_R"){
        if(a->type() == "O_2" || c->type() == "O_2" || d->type() == "O_2"){
            k = 50;
        }
        else{
            k = 6;
        }

        c0 = 1;
        c1 = -1;
        c2 = 0;
    }

    // divide by 3
    k /= 3;

    setParameter(0, k);
    setParameter(1, c0);
    setParameter(2, c1);
    setParameter(3, c2);

    return true;
}

chemkit::Float UffInversionCalculation::energy() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);
    const chemkit::ForceFieldAtom *d = atom(3);

    chemkit::Float k = parameter(0);
    chemkit::Float c0 = parameter(1);
    chemkit::Float c1 = parameter(2);
    chemkit::Float c2 = parameter(3);

    chemkit::Float w = wilsonAngleRadians(a, b, c, d);
    chemkit::Float y = w + (chemkit::constants::Pi / 2.0);

    return k * (c0 + c1 * sin(y) + c2 * cos(2 * y));
}

std::vector<chemkit::Vector3> UffInversionCalculation::gradient() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);
    const chemkit::ForceFieldAtom *c = atom(2);
    const chemkit::ForceFieldAtom *d = atom(3);

    chemkit::Float k = parameter(0);
    chemkit::Float c1 = parameter(2);
    chemkit::Float c2 = parameter(3);

    chemkit::Float w = wilsonAngleRadians(a, b, c, d);
    chemkit::Float y = w + (chemkit::constants::Pi / 2.0);

    // dE/dw
    chemkit::Float de_dw = k * (c1 * cos(y) - 2 * c2 * sin(2 * y));

    std::vector<chemkit::Vector3> gradient = wilsonAngleGradientRadians(a, b, c, d);

    gradient[0] *= de_dw;
    gradient[1] *= de_dw;
    gradient[2] *= de_dw;
    gradient[3] *= de_dw;

    return gradient;
}

// === UffVanDerWaalsCalculation =========================================== //
UffVanDerWaalsCalculation::UffVanDerWaalsCalculation(const chemkit::ForceFieldAtom *a,
                                                     const chemkit::ForceFieldAtom *b)
    : UffCalculation(VanDerWaals, 2, 2)
{
    setAtom(0, a);
    setAtom(1, b);
}

bool UffVanDerWaalsCalculation::setup()
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);

    const UffAtomParameters *pa = parameters(a);
    const UffAtomParameters *pb = parameters(b);

    if(!pa || !pb){
        return false;
    }

    // equation 22
    chemkit::Float d = sqrt(pa->D * pb->D);

    // equation 21b
    chemkit::Float x = sqrt(pa->x * pb->x);

    setParameter(0, d);
    setParameter(1, x);

    return true;
}

chemkit::Float UffVanDerWaalsCalculation::energy() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);

    chemkit::Float d = parameter(0);
    chemkit::Float x = parameter(1);
    chemkit::Float r = distance(a, b);

    return d * (-2 * pow(x/r, 6) + pow(x/r, 12));
}

std::vector<chemkit::Vector3> UffVanDerWaalsCalculation::gradient() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);

    chemkit::Float d = parameter(0);
    chemkit::Float x = parameter(1);
    chemkit::Float r = distance(a, b);

    // dE/dr
    chemkit::Float de_dr = -12 * d * x / pow(r, 2) * (pow(x/r, 11) - pow(x/r, 5));

    std::vector<chemkit::Vector3> gradient = distanceGradient(a, b);

    gradient[0] *= de_dr;
    gradient[1] *= de_dr;

    return gradient;
}

// === UffElectrostaticCalculation ========================================= //
UffElectrostaticCalculation::UffElectrostaticCalculation(const chemkit::ForceFieldAtom *a,
                                                         const chemkit::ForceFieldAtom *b)
    : UffCalculation(Electrostatic, 2, 2)
{
    setAtom(0, a);
    setAtom(1, b);
}

bool UffElectrostaticCalculation::setup()
{
    return false;
}

chemkit::Float UffElectrostaticCalculation::energy() const
{
    const chemkit::ForceFieldAtom *a = atom(0);
    const chemkit::ForceFieldAtom *b = atom(1);

    chemkit::Float qa = parameter(0);
    chemkit::Float qb = parameter(1);

    chemkit::Float e = 1;
    chemkit::Float r = distance(a, b);

    return 332.037 * (qa * qb) / (e * r);
}
