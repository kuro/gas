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


#pragma once

#include <QMainWindow>
#include "ui_EditWindow.h"

#include <QTimer>

#include <gas/io.h>
#include <gas/qt/context.h>

#include "GasTreeModel.h"
#include "GasTableModel.h"

class EditWindow : public QMainWindow, private Ui::EditWindow
{
    Q_OBJECT

private:
    GAScontext* qt_file_context;
    GASio* gio;
    GASchunk* current_chunk;
    GasTreeModel *tree_model;

    QItemSelectionModel* tree_selection_model;

    QTimer payload_timer;
    bool ignore_next_payload_text_changed;

    bool ignore_changes;

public:
    EditWindow ();
    virtual ~EditWindow ();

private:
    void load (QString fname);

private slots:
    void on_tree_selection_model_currentChanged (const QModelIndex&,
                                                 const QModelIndex&);
    void on_attribute_table_cellChanged (int, int);

    void on_id_line_edit_textChanged ();
    void on_payload_text_edit_textChanged ();

    void on_payload_timer_timeout ();
};
