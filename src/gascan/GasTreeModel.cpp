/*
 * Copyright 2008 Blanton Black
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

#include "GasTreeModel.moc"

#include <gas/tree.h>

#include <QStringList>
#include <QtCore>

using namespace Gas;

/* GasTreeModel() {{{*/
GasTreeModel::GasTreeModel () :
    root(NULL)
{
}
/*}}}*/
/* ~GasTreeModel() {{{*/
GasTreeModel::~GasTreeModel ()
{
}
/*}}}*/
/* columnCount() {{{*/
int GasTreeModel::columnCount (const QModelIndex &parent) const
{
    return 4;
}
/*}}}*/
/* flags() {{{*/
Qt::ItemFlags GasTreeModel::flags (const QModelIndex &index) const
{
    //qDebug("flags");
    if (! index.isValid()) {
        return 0;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
/*}}}*/
/* headerData() {{{*/
QVariant GasTreeModel::headerData (int section, Qt::Orientation orientation,
        int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Chunk");
        case 1:
            return tr("Attributes");
        case 2:
            return tr("Payload");
        case 3:
            return tr("Size");
        default:
            return QVariant();
        }
    }
    return QVariant();
}
/*}}}*/

/* data() {{{*/
QVariant GasTreeModel::data (const QModelIndex &index, int role) const
{
    if (! index.isValid()) {
        return QVariant();
    }

    GASchunk *c = static_cast<GASchunk*>(index.internalPointer());

    switch (index.column()) {
    case 0:
        switch (role) {
        case Qt::ToolTipRole:
            return QByteArray((char*)c->id, c->id_size)
                + " (" + QString::number(gas_total_size(c)) + ")";
        case Qt::DisplayRole:
            return QByteArray((char*)c->id, c->id_size);
        default:
            return QVariant();
        }
    case 1:
        switch (role) {
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
            {
                QStringList attributes;
                for (GASunum i = 0; i < c->nb_attributes; i++) {
                    Attribute& attr = c->attributes[i];
                    attributes
                        << (QString("\"")
                            + QByteArray((char*)attr.key, attr.key_size)
                            + "\" => \""
                            + QByteArray((char*)attr.value, attr.value_size)
                            + "\""
                           )
                        ;

                }
                return attributes.join(", ");
            }
        default:
            return QVariant();
        }
    case 2:
        switch (role) {
        case Qt::ToolTipRole:
            return QByteArray((char*)c->payload, c->payload_size)
                + " (" + QString::number(gas_total_size(c)) + ")";
        case Qt::DisplayRole:
            return QByteArray((char*)c->payload, c->payload_size);
        default:
            return QVariant();
        }
    case 3:
        switch (role) {
        case Qt::DisplayRole:
            return (unsigned int)gas_total_size(c);
        default:
            return QVariant();
        }
    default:
        return QVariant();
    }
}
/*}}}*/
/* rowCount() {{{*/
int GasTreeModel::rowCount (const QModelIndex &parent) const
{
    GASchunk *parent_chunk;

    if (root == NULL) {
        return 0;
    }

    if (parent.column() > 0) {
        return 0;
    }

    if (parent.isValid()) {
        parent_chunk = static_cast<GASchunk*>(parent.internalPointer());
    } else {
        parent_chunk = root;
        //return 0;  // FIXME
    }

    return parent_chunk->nb_children;
}
/*}}}*/
/* index() {{{*/
QModelIndex GasTreeModel::index (int row, int col,
        const QModelIndex &parent) const
{
    GASchunk *parent_chunk;

    if (root == NULL) {
        return QModelIndex();
    }

    if ( ! hasIndex(row, col, parent)) {
        return QModelIndex();
    }

    if (parent.isValid()) {
        parent_chunk = static_cast<GASchunk*>(parent.internalPointer());
    } else {
        parent_chunk = root;
    }

    if ((GASunum)row < parent_chunk->nb_children) {
        return createIndex(row, col, parent_chunk->children[row]);
    } else {
        return QModelIndex();
    }
}
/*}}}*/
/* parent() {{{*/
QModelIndex GasTreeModel::parent (const QModelIndex &child) const
{
    if (! child.isValid()) {
        return QModelIndex();
    }
   
    GASchunk *child_chunk = static_cast<GASchunk*>(child.internalPointer());
    GASchunk *parent_chunk = child_chunk->parent;

    //qDebug("parent for %s", (char*)child_chunk->id);

    if (parent_chunk == root) {
        return QModelIndex();
    }

    // determine the parent's row
    int row;
    if (parent_chunk->parent == NULL) {
        row = 0;
    } else {
        for (unsigned int i = 0; i < parent_chunk->parent->nb_children; i++) {
            if (parent_chunk->parent->children[i] == parent_chunk) {
                row = i;
                break;
            }
        }
    }

    return createIndex(row, 0, parent_chunk);
}
/*}}}*/

void GasTreeModel::setRoot (GASchunk* c)
{
    root = c;
    reset();
}

// vim: sw=4 fdm=marker
