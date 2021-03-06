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

#include "graphicsmolecularsurfaceitem.h"

#include <chemkit/molecule.h>
#include <chemkit/alphashape.h>
#include <chemkit/quaternion.h>

#include "graphicsscene.h"
#include "graphicssphere.h"
#include "graphicspainter.h"
#include "graphicsmaterial.h"
#include "graphicsmoleculeitem.h"
#include "graphicsvertexbuffer.h"

namespace chemkit {

namespace {

// === ClippedSphere ======================================================= //
class ClippedSphere
{
    public:
        ClippedSphere(float radius);

        void addClipPlane(const Point3f &point, const Vector3f &normal);
        GraphicsVertexBuffer* tesselate() const;

    private:
        float m_radius;
        QList<QPair<Point3f, Vector3f> > m_clipPlanes;
};

ClippedSphere::ClippedSphere(float radius)
    : m_radius(radius)
{
}

void ClippedSphere::addClipPlane(const Point3f &point, const Vector3f &normal)
{
    m_clipPlanes.append(qMakePair(point, normal));
}

GraphicsVertexBuffer* ClippedSphere::tesselate() const
{
    GraphicsVertexBuffer *buffer = GraphicsSphere(m_radius).tesselate();

    QVector<Point3f> verticies = buffer->verticies();
    QVector<Vector3f> normals = buffer->normals();
    QVector<unsigned short> indicies = buffer->indicies();

    QVector<unsigned short> clippedIndicies;

    for(int triangleIndex = 0; triangleIndex < indicies.size() / 3; triangleIndex++){
        unsigned short ia = indicies[triangleIndex*3+0];
        unsigned short ib = indicies[triangleIndex*3+1];
        unsigned short ic = indicies[triangleIndex*3+2];

        const Point3f &a = verticies[ia];
        const Point3f &b = verticies[ib];
        const Point3f &c = verticies[ic];

        // check triangle against each clipping plane
        bool clipTriangle = false;

        for(int clipPlaneIndex = 0; clipPlaneIndex < m_clipPlanes.size(); clipPlaneIndex++){
            const Point3f &planePoint = m_clipPlanes[clipPlaneIndex].first;
            const Vector3f &planeNormal = m_clipPlanes[clipPlaneIndex].second;

            QList<unsigned short> invalidVerticies;

            if((planePoint - a).dot(planeNormal) < 0){
                invalidVerticies.append(ia);
            }
            if((planePoint - b).dot(planeNormal) < 0){
                invalidVerticies.append(ib);
            }
            if((planePoint - c).dot(planeNormal) < 0){
                invalidVerticies.append(ic);
            }

            // keep entire triangle
            if(invalidVerticies.isEmpty()){
                continue;
            }
            // clip entire triangle
            else if(invalidVerticies.size() == 3){
                clipTriangle = true;
                break;
            }
            // clip part of the triangle
            else{
                foreach(unsigned short vertexIndex, invalidVerticies){
                    Point3f invalidPoint = verticies[vertexIndex];

                    float d = -(planePoint - invalidPoint).dot(planeNormal);
                    float theta = acos(planePoint.norm() / m_radius) - acos((planePoint.norm() + d) / m_radius);
                    Vector3f up = invalidPoint.cross(planeNormal).normalized();

                    // set new vertex position
                    verticies[vertexIndex] = Quaternionf::rotateRadians(invalidPoint, up, -theta);

                    // update normal
                    normals[vertexIndex] = verticies[vertexIndex].normalized();
                }
            }
        }

        if(!clipTriangle){
            clippedIndicies.append(ia);
            clippedIndicies.append(ib);
            clippedIndicies.append(ic);
        }
    }

    buffer->setVerticies(verticies);
    buffer->setNormals(normals);
    buffer->setIndicies(clippedIndicies);

    return buffer;
}

// === ContactPatchItem ==================================================== //
class ContactPatchItem : public GraphicsItem
{
    public:
        ContactPatchItem(GraphicsMolecularSurfaceItem *parent, const Point3f &center, float radius);
        ~ContactPatchItem();

        Point3f center() const;
        float radius() const;
        void setColor(const QColor &color);
        void addIntersection(const ContactPatchItem *item);

        void paint(GraphicsPainter *painter);

    private:
        GraphicsMolecularSurfaceItem *m_parent;
        Point3f m_center;
        float m_radius;
        QColor m_color;
        GraphicsVertexBuffer *m_buffer;
        QList<const ContactPatchItem *> m_intersections;
};

ContactPatchItem::ContactPatchItem(GraphicsMolecularSurfaceItem *parent, const Point3f &center, float radius)
    : GraphicsItem(),
      m_parent(parent),
      m_center(center),
      m_radius(radius)
{
    m_buffer = 0;
    m_color = Qt::red;

    translate(center);
}

ContactPatchItem::~ContactPatchItem()
{
    delete m_buffer;
}

Point3f ContactPatchItem::center() const
{
    return m_center;
}

float ContactPatchItem::radius() const
{
    return m_radius;
}

void ContactPatchItem::setColor(const QColor &color)
{
    m_color = color;
}

void ContactPatchItem::addIntersection(const ContactPatchItem *item)
{
    m_intersections.append(item);
}

void ContactPatchItem::paint(GraphicsPainter *painter)
{
    if(!m_buffer){
        ClippedSphere clippedSphere(m_radius);

        // calculate and add clip plane for each intersection
        foreach(const ContactPatchItem *item, m_intersections){
            const Point3f &a = m_center;
            float ra = m_radius;
            const Point3f &b = item->center();
            float rb = item->radius();

            const float d = a.distance(b);
            const float x = (d*d - rb*rb + ra*ra) / (2 * d);

            Vector3f planeNormal = (b - a).normalized();
            const Point3f planeCenter = planeNormal * x;

            clippedSphere.addClipPlane(planeCenter, planeNormal);
        }

        m_buffer = clippedSphere.tesselate();
    }

    QColor color = m_color;
    color.setAlphaF(opacity());
    painter->setColor(color);

    painter->setMaterial(m_parent->material());

    painter->draw(m_buffer);
}

} // end anonymous namespace

// === GraphicsMolecularSurfaceItemPrivate ================================= //
class GraphicsMolecularSurfaceItemPrivate
{
    public:
        MolecularSurface *surface;
        QColor color;
        GraphicsAtomColorMap *colorMap;
        GraphicsMolecularSurfaceItem::ColorMode colorMode;
        QList<ContactPatchItem *> contactPatches;
};

// === GraphicsMolecularSurfaceItem ======================================== //
/// \class GraphicsMolecularSurfaceItem graphicsmolecularsurfaceitem.h chemkit/graphicsmolecularsurfaceitem.h
/// \ingroup chemkit-graphics
/// \brief The GraphicsMolecularSurfaceItem displays a molecular
///        surface.
///
/// The image below shows a molecular surface item on top of a
/// guanine molecule:
/// \image html molecular-surface-item.png
///
/// \see MolecularSurface

/// \enum GraphicsMolecularSurfaceItem::ColorMode
/// Provides different modes for coloring the surface.
///     - \c SolidColor
///     - \c AtomColor

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new molecular surface item for \p molecule.
GraphicsMolecularSurfaceItem::GraphicsMolecularSurfaceItem(const Molecule *molecule)
    : GraphicsItem(),
      d(new GraphicsMolecularSurfaceItemPrivate)
{
    d->surface = new MolecularSurface(molecule, MolecularSurface::SolventExcluded);
    d->color = Qt::red;
    d->colorMode = AtomColor;
    d->colorMap = new GraphicsAtomColorMap(GraphicsAtomColorMap::DefaultColorScheme);

    setMolecule(molecule);
}

/// Creates a new molecular surface item for \p surface.
GraphicsMolecularSurfaceItem::GraphicsMolecularSurfaceItem(const MolecularSurface *surface)
    : GraphicsItem(),
      d(new GraphicsMolecularSurfaceItemPrivate)
{
    d->surface = new MolecularSurface(surface->molecule(), MolecularSurface::SolventExcluded);
    d->color = Qt::red;
    d->colorMode = AtomColor;
    d->colorMap = new GraphicsAtomColorMap(GraphicsAtomColorMap::DefaultColorScheme);

    setSurface(surface);
}

/// Destroys the molecular surface item.
GraphicsMolecularSurfaceItem::~GraphicsMolecularSurfaceItem()
{
    delete d->surface;
    delete d->colorMap;
    delete d;
}

// --- Properties ---------------------------------------------------------- //
/// Sets the surface to display to \p surface.
void GraphicsMolecularSurfaceItem::setSurface(const MolecularSurface *surface)
{
    if(surface){
        d->surface->setMolecule(surface->molecule());
        d->surface->setSurfaceType(surface->surfaceType());
        d->surface->setProbeRadius(surface->probeRadius());
    }
    else{
        d->surface->setMolecule(0);
    }

    recalculate();
}

/// Returns the surface that the item is displaying.
const MolecularSurface* GraphicsMolecularSurfaceItem::surface() const
{
    return d->surface;
}

/// Sets the molecule for the surface to \p molecule.
void GraphicsMolecularSurfaceItem::setMolecule(const Molecule *molecule)
{
    d->surface->setMolecule(molecule);

    recalculate();
}

/// Returns the molecule for the surface.
const Molecule* GraphicsMolecularSurfaceItem::molecule() const
{
    return d->surface->molecule();
}

/// Sets the surface type to \p type.
void GraphicsMolecularSurfaceItem::setSurfaceType(MolecularSurface::SurfaceType type)
{
    d->surface->setSurfaceType(type);

    recalculate();
}

/// Returns the surface type.
MolecularSurface::SurfaceType GraphicsMolecularSurfaceItem::surfaceType() const
{
    return d->surface->surfaceType();
}

/// Sets the probe radius for the surface to \p radius.
void GraphicsMolecularSurfaceItem::setProbeRadius(float radius)
{
    d->surface->setProbeRadius(radius);

    if(surfaceType() == MolecularSurface::SolventAccessible ||
       surfaceType() == MolecularSurface::SolventExcluded){
        recalculate();
    }
}

/// Returns the probe radius for the surface.
float GraphicsMolecularSurfaceItem::probeRadius() const
{
    return d->surface->probeRadius();
}

/// Sets the color for the surface to \p color.
void GraphicsMolecularSurfaceItem::setColor(const QColor &color)
{
    d->color = color;

    if(d->colorMode == SolidColor){
        foreach(ContactPatchItem *item, d->contactPatches){
            item->setColor(d->color);
        }
    }
}

/// Returns the color for the surface.
QColor GraphicsMolecularSurfaceItem::color() const
{
    return d->color;
}

/// Sets the color mode for the surface item to \p mode.
void GraphicsMolecularSurfaceItem::setColorMode(ColorMode mode)
{
    d->colorMode = mode;

    if(d->colorMode == SolidColor){
        foreach(ContactPatchItem *item, d->contactPatches){
            item->setColor(d->color);
        }
    }
    else if(d->colorMode == AtomColor){
        for(int i = 0; i < d->contactPatches.size(); i++){
            ContactPatchItem *item = d->contactPatches[i];
            item->setColor(d->colorMap->color(molecule()->atom(i)));
        }
    }
}

/// Returns the color mode for the surface item.
GraphicsMolecularSurfaceItem::ColorMode GraphicsMolecularSurfaceItem::colorMode() const
{
    return d->colorMode;
}

/// Sets the color map for the surface item to \p colorMap.
void GraphicsMolecularSurfaceItem::setAtomColorMap(GraphicsAtomColorMap *colorMap)
{
    delete d->colorMap;

    d->colorMap = colorMap;
}

/// Returns the color map for the surface item.
GraphicsAtomColorMap* GraphicsMolecularSurfaceItem::colorMap() const
{
    return d->colorMap;
}

// --- Internal Methods ---------------------------------------------------- //
void GraphicsMolecularSurfaceItem::itemChanged(ItemChange change)
{
    if(change == ItemVisiblityChanged){
        foreach(ContactPatchItem *item, d->contactPatches){
            item->setVisible(isVisible());
        }
    }
    else if(change == ItemOpacityChanged){
        foreach(ContactPatchItem *item, d->contactPatches){
            item->setOpacity(opacity());
        }

        if(isOpaque()){
            material()->setSpecularColor(QColor::fromRgbF(0.3, 0.3, 0.3));
        }
        else{
            material()->setSpecularColor(Qt::transparent);
        }
    }
    else if(change == ItemSceneChanged){
        foreach(ContactPatchItem *item, d->contactPatches){
            if(scene()){
                scene()->addItem(item);
            }
        }
    }
}

void GraphicsMolecularSurfaceItem::recalculate()
{
    qDeleteAll(d->contactPatches);
    d->contactPatches.clear();

    const Molecule *molecule = this->molecule();
    if(!molecule){
        return;
    }

    // create contact patches
    foreach(const Atom *atom, molecule->atoms()){
        float radius = atom->vanDerWaalsRadius();
        if(surfaceType() == MolecularSurface::SolventAccessible){
            radius += probeRadius();
        }

        ContactPatchItem *item = new ContactPatchItem(this, atom->position(), radius);
        d->contactPatches.append(item);

        if(d->colorMode == AtomColor){
            item->setColor(d->colorMap->color(atom));
        }
        else{
            item->setColor(d->color);
        }

        item->setOpacity(opacity());

        if(scene()){
            scene()->addItem(item);
        }
    }

    // add intersections from alpha shape edges
    const AlphaShape *alphaShape = d->surface->alphaShape();

    foreach(const std::vector<int> &edge, alphaShape->edges()){
        ContactPatchItem *itemA = d->contactPatches[edge[0]];
        ContactPatchItem *itemB = d->contactPatches[edge[1]];

        itemA->addIntersection(itemB);
        itemB->addIntersection(itemA);
    }
}

} // end chemkit namespace
