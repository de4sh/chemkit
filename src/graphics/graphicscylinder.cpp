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

#include "graphicscylinder.h"

#include "graphicsray.h"
#include "graphicsvertexbuffer.h"

namespace chemkit {

// === GraphicsCylinder ==================================================== //
/// \class GraphicsCylinder graphicscylinder.h chemkit/graphicscylinder.h
/// \ingroup chemkit-graphics
/// \internal
/// \brief The GraphicsCylinder class represents a cylinder.

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new cylinder with radius of \c 0 and length of \c 0.
GraphicsCylinder::GraphicsCylinder()
{
    m_radius = 0;
    m_length = 0;
}

/// Create a new cylinder object with \p radius and \p length.
GraphicsCylinder::GraphicsCylinder(float radius, float length)
{
    m_radius = radius;
    m_length = length;
}

/// Destroys the cylinder object.
GraphicsCylinder::~GraphicsCylinder()
{
}

// --- Properties ---------------------------------------------------------- //
/// Sets the radius of the cylinder to \p radius.
void GraphicsCylinder::setRadius(float radius)
{
    m_radius = radius;
}

/// Returns the radius of the cylinder.
float GraphicsCylinder::radius() const
{
    return m_radius;
}

/// Sets the length of the cylinder to \p length.
void GraphicsCylinder::setLength(float length)
{
    m_length = length;
}

/// Returns the length of the cylinder.
float GraphicsCylinder::length() const
{
    return m_length;
}

// --- Intersection -------------------------------------------------------- //
bool GraphicsCylinder::intersects(const GraphicsRay &ray, float *distance) const
{
    Q_UNUSED(ray);
    Q_UNUSED(distance);

    return false;
}

// --- Tesselation --------------------------------------------------------- //
GraphicsVertexBuffer* GraphicsCylinder::tesselate(int slices, int stacks) const
{
    // slices must be at least 3
    slices = qMax(3, slices);

    // stacks must be at least 1
    stacks = qMax(1, stacks);

    float twoPi = chemkit::constants::Pi * 2;
    float sliceAngle = twoPi / slices;

    QVector<Point3f> verticies;

    for(int i = 0; i < slices; i++){
        float angle = i * sliceAngle;

        float x = cos(angle) * m_radius;
        float y = sin(angle) * m_radius;

        verticies.append(Point3f(x, y, 0));
    }

    Float stackHeight = m_length / stacks;

    for(int i = 1; i < stacks + 1; i++){
        for(int j = 0; j < slices; j++){
            Point3f lowerPoint = verticies[(i-1)*slices + j];

            verticies.append(lowerPoint.movedBy(0, 0, stackHeight));
        }
    }

    QVector<Vector3f> normals;
    foreach(const Point3f &point, verticies){
        Vector3f normal(point.x(), point.y(), 0);
        normal.normalize();
        normals.append(normal);
    }

    QVector<unsigned short> indicies;
    for(int i = 0; i < stacks; i++){
        for(int j = 0; j < slices; j++){
            // triangle indices
            unsigned short i0, i1, i2;

            // first triangle
            i0 = i * slices + j;
            i1 = i * slices + ((j + 1) % slices);
            i2 = (i+1) * slices + j;

            indicies.append(i0);
            indicies.append(i1);
            indicies.append(i2);

            // second triangle
            i0 = i1;
            i1 = (i+1)*slices + ((j + 1) % slices);

            indicies.append(i0);
            indicies.append(i1);
            indicies.append(i2);
        }
    }

    GraphicsVertexBuffer *buffer = new GraphicsVertexBuffer;

    buffer->setVerticies(verticies);
    buffer->setNormals(normals);
    buffer->setIndicies(indicies);

    return buffer;
}

} // end chemkit namespace
