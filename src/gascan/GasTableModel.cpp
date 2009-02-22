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

#include "GasTableModel.moc"
#include "GasTreeModel.h"

GasTableModel::GasTableModel () :
    chunk(NULL)
{
}

GasTableModel::~GasTableModel ()
{
}

void GasTableModel::load (GASchunk* chunk)
{
    this->chunk = chunk;
    reset();
}

int GasTableModel::rowCount (const QModelIndex& parent) const
{
    if (chunk == NULL) {
        return 0;
    }
    return chunk->nb_attributes;
}

int GasTableModel::columnCount (const QModelIndex& parent) const
{
    return 2;
}


QVariant GasTableModel::headerData (int section, Qt::Orientation orientation,/*{{{*/
                     int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Key");
        case 1:
            return tr("Value");
        default:
            return QVariant();
        }
    }
    return QVariant();
}/*}}}*/

QVariant GasTableModel::data (const QModelIndex& index, int role) const
{
    GASattribute* attr = &chunk->attributes[index.row()];

    switch (index.column())
    {
    case 0:
        switch (role)
        {
        case Qt::ToolTipRole:
            return QByteArray (reinterpret_cast<char*>(attr->key),
                               attr->key_size);
        case Qt::DisplayRole:
            return QByteArray (reinterpret_cast<char*>(attr->key),
                               attr->key_size);
        default:
            return QVariant();
        };
    case 1:
        switch (role)
        {
        case Qt::ToolTipRole:
            return QByteArray (reinterpret_cast<char*>(attr->value),
                               attr->value_size);
        case Qt::DisplayRole:
            return QByteArray (reinterpret_cast<char*>(attr->value),
                               attr->value_size);
        default:
            return QVariant();
        };
    default:
        return QVariant();
    }


}

// vim: sw=4 fdm=marker
