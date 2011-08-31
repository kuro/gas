/*
 * Copyright 2011 Blanton Black
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file GasModel.cpp
 * @brief GasModel implementation
 */

#include "GasModel.moc"

#include "Chunk.h"

using namespace Gas;

static
Chunk* chunk_cast (void* p)
{
    return qobject_cast<Chunk*>(static_cast<QObject*>(p));
}

struct GasModel::Private
{
    Chunk* root;
};

GasModel::GasModel (Chunk* root, QObject* parent) :
    QAbstractItemModel(parent),
    d(new Private)
{
    d->root = new Chunk("invisible_root");
    root->setParentChunk(d->root);  ///< @todo i don't like having to do this
}

GasModel::~GasModel ()
{
    /// @todo clean up: complicated by taking over the chunk
    delete d;
}

QModelIndex GasModel::index (int row, int col, const QModelIndex& parent) const
{
    if (!hasIndex(row, col, parent)) {
        return QModelIndex();
    }

    Chunk* p;

    if (!parent.isValid()) {
        p = d->root;
    } else {
        p = chunk_cast(parent.internalPointer());
    }
    Q_ASSERT(p);

    Chunk* c = p->childChunks().at(row);
    return createIndex(row, col, c);
}

QModelIndex GasModel::parent (const QModelIndex& idx) const
{
    if (!idx.isValid()) {
        return QModelIndex();
    }

    Chunk* c = chunk_cast(idx.internalPointer());
    Q_ASSERT(c);
    Chunk* p = c->parentChunk();

    if (p == d->root) {
        return QModelIndex();
    }

    return createIndex(p->parentChunk()->childChunks().indexOf(p), 0, p);
}

int GasModel::rowCount (const QModelIndex& parent) const
{
    if (parent.column() > 0) {
        return 0;
    }
    Chunk* p;
    if (!parent.isValid()) {
        p = d->root;
    } else {
        p = chunk_cast(parent.internalPointer());
    }
    Q_ASSERT(p);
    return p->childChunks().size();
}

int GasModel::columnCount (const QModelIndex& parent) const
{
    return 1;
}

QVariant GasModel::data (const QModelIndex& idx, int role) const
{
    Chunk* c = NULL;
    switch (role) {
    case Qt::DisplayRole: {
        if ((c = chunk_cast(idx.internalPointer()))) {
            return c->id();
        }
        return QVariant();
    }
    default:
        return QVariant();
    }
}
