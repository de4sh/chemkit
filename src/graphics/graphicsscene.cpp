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

#include "graphicsscene.h"

#include "graphicsitem.h"
#include "graphicsview.h"

namespace chemkit {

// === GraphicsScenePrivate ================================================ //
class GraphicsScenePrivate
{
    public:
        QList<GraphicsItem *> items;
        QList<GraphicsView *> views;
};

// === GraphicsScene ======================================================= //
/// \class GraphicsScene graphicsscene.h chemkit/graphicsscene.h
/// \ingroup chemkit-graphics
/// \brief The GraphicsScene class contains graphics items.
///
/// The GraphicsScene class contains and organizes GraphicsItems.
///
/// To display a graphics scene use the GraphicsView class.

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new graphics scene.
GraphicsScene::GraphicsScene()
    : d(new GraphicsScenePrivate)
{
}

/// Destroys the graphics scene.
GraphicsScene::~GraphicsScene()
{
    foreach(GraphicsItem *item, d->items){
        deleteItem(item);
    }

    delete d;
}

// --- Properties ---------------------------------------------------------- //
/// Returns the number of items in the scene.
int GraphicsScene::size() const
{
    return itemCount();
}

/// Returns \c true if the scene contains no items.
bool GraphicsScene::isEmpty() const
{
    return size() == 0;
}

/// Returns a list of views that show the scene.
QList<GraphicsView *> GraphicsScene::views() const
{
    return d->views;
}

// --- Items --------------------------------------------------------------- //
/// Adds \p item to the scene.
///
/// The scene takes ownership of the item.
void GraphicsScene::addItem(GraphicsItem *item)
{
    d->items.append(item);
    item->setScene(this);
}

/// Removes \p item from the scene. Returns \c true if the item was
/// found and removed successfully.
///
/// The ownership of item is passed to the caller.
bool GraphicsScene::removeItem(GraphicsItem *item)
{
    bool found = d->items.removeOne(item);

    if(found){
        item->setScene(0);
    }

    return found;
}

/// Removes \p item from the scene and deletes it. Returns \c true
/// if the item was found and removed successfully.
bool GraphicsScene::deleteItem(GraphicsItem *item)
{
    bool found = removeItem(item);

    if(found){
        delete item;
    }

    return found;
}

/// Returns the item at \p index.
GraphicsItem* GraphicsScene::item(int index) const
{
    return d->items.value(index, 0);
}

/// Returns the item that intersects \p ray.
GraphicsItem* GraphicsScene::item(const GraphicsRay &ray) const
{
    GraphicsItem *closestItem = 0;
    float closestDistance = qInf();

    foreach(GraphicsItem *item, d->items){
        float distance;

        if(item->intersects(ray, &distance)){
            if(!closestItem || distance < closestDistance){
                closestItem = item;
                closestDistance = distance;
            }
        }
    }

    return closestItem;
}

/// Returns a list of items in the scene.
QList<GraphicsItem *> GraphicsScene::items() const
{
    return d->items;
}

/// Returns a list of all items that intersect \p ray.
QList<GraphicsItem *> GraphicsScene::items(const GraphicsRay &ray, bool sorted) const
{
    QList<GraphicsItem *> items;

    if(sorted){
        QList<float> distances;

        foreach(GraphicsItem *item, d->items){
            float distance;

            if(item->intersects(ray, &distance)){
                int index;
                for(index = 0; index < items.size(); index++)
                    if(distance < distances[index])
                        break;

                items.insert(index, item);
                distances.insert(index, distance);
            }
        }
    }
    else{
        foreach(GraphicsItem *item, d->items){
            if(item->intersects(ray)){
                items.append(item);
            }
        }
    }

    return items;
}

/// Returns the number of items in the scene.
int GraphicsScene::itemCount() const
{
    return d->items.size();
}

// --- Internal Methods ---------------------------------------------------- //
void GraphicsScene::addView(GraphicsView *view)
{
    d->views.append(view);
}

void GraphicsScene::removeView(GraphicsView *view)
{
    d->views.removeOne(view);
}

} // end chemkit namespace
