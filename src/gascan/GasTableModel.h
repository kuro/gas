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

#include <QAbstractTableModel>

#include <gas/tree.h>

class GasTableModel : public  QAbstractTableModel
{
    Q_OBJECT

private:
    GASchunk* chunk;

public:
    GasTableModel ();
    virtual ~GasTableModel ();

    void load (GASchunk* chunk);

    int rowCount (const QModelIndex &parent = QModelIndex()) const;
    int columnCount (const QModelIndex &parent = QModelIndex()) const;

    QVariant data (const QModelIndex &index, int role) const;

    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;
};

// vim: sw=4 fdm=marker
