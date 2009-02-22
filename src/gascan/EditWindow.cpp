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


#include "EditWindow.moc"

#include <QtCore>

#include <gas/fsio.h>
#include <gas/bufio.h>

EditWindow::EditWindow () :/*{{{*/
    QMainWindow(),
    tree_selection_model(NULL),
    ignore_next_payload_text_changed(true),
    ignore_changes(true)
{
    setupUi(this);

    qt_file_context = gas_new_qiodevice_context();
    gas_io_new(&gio, qt_file_context);

//    GASchunk *c0, *c10, *c11;
//    gas_new_named(&c0, "c0");
//    gas_new_named(&c10, "c10");
//    gas_new_named(&c11, "c11");
//    gas_add_child(c0, c10);
//    gas_add_child(c0, c11);
//
//    gas_set_attribute_ss(c0, "c0", "0c");
//    gas_set_attribute_ss(c10, "a", "1");
//    gas_set_attribute_ss(c10, "b", "2");
//    gas_set_payload_s(c10, "hello world");
//    gas_set_attribute_ss(c11, "a", "1");
//    gas_set_attribute_ss(c11, "b", "2");
//    gas_set_attribute_ss(c11, "c", "3");
//
//    QtConcurrent::run(gas_update, c0);


    tree_model = new GasTreeModel;
    treeView->setModel(tree_model);

    tree_model->root = NULL;
    load("data2.gas");
    //load("alerts.gas");

    tree_selection_model = new QItemSelectionModel(tree_model, this); 
    connect(tree_selection_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(on_tree_selection_model_currentChanged(QModelIndex, QModelIndex)));
    treeView->setSelectionModel(tree_selection_model);

    payload_timer.setInterval(10000);
    payload_timer.setSingleShot(true);
    connect(&payload_timer, SIGNAL(timeout()),
            this, SLOT(on_payload_timer_timeout()));
}/*}}}*/

EditWindow::~EditWindow ()/*{{{*/
{
}/*}}}*/

void EditWindow::load (QString fname)
{
    GASresult gr;
    GASchunk* c;

    QTime t0 = QTime::currentTime();
#if 0
    QFile file (fname);
    file.open(QIODevice::ReadOnly);
    gas_io_set_handle(gio, &file);

    gr = gas_read_io(gio, &c);
    file.close();
#endif
#if 0
    FILE* fs;
    fs = fopen(qPrintable(fname), "rb");
    gr = gas_read_fs(fs, &c);
#endif
#if 1
    QFileInfo finfo (fname);
    QFile file (fname);
    file.open(QIODevice::ReadOnly);
    uchar* buf = file.map(0, finfo.size());
    if (buf == NULL) {
        qCritical() << "failed to map file";
        return;
    }
    gr = gas_read_buf(buf, finfo.size(), &c);
    file.close();
#endif

    qDebug() << "parse time: " << t0.secsTo(QTime::currentTime());

    if (gr < GAS_OK) {
        qDebug() << gr << gas_error_string(gr);
        return;
    }

    gas_new_named(&tree_model->root, "dummy");
    gas_add_child(tree_model->root, c);

    QtConcurrent::run(gas_update, c);
}

void EditWindow::on_tree_selection_model_currentChanged (/*{{{*/
        const QModelIndex& cur, const QModelIndex& prev
        )
{
    // cleanup current chunk
    if (payload_timer.isActive()) {
        qDebug() << "flushing";
        payload_timer.stop();
        on_payload_timer_timeout();
    }

    // load new chunk
    current_chunk = static_cast<GASchunk*>(cur.internalPointer());

    ignore_changes = true;

    // id line
    id_line_edit->setText(QByteArray((char*)current_chunk->id,
                                     current_chunk->id_size));

    // attribute table
    attribute_table->setRowCount(current_chunk->nb_attributes);
    GASattribute* attr = NULL;
    for (unsigned int i = 0; i < current_chunk->nb_attributes; i++) {
        attr = &current_chunk->attributes[i];
        QTableWidgetItem* key_item =
            new QTableWidgetItem(QByteArray((char*)attr->key, attr->key_size),
                    QTableWidgetItem::Type);
        QTableWidgetItem* val_item =
            new QTableWidgetItem(QByteArray((char*)attr->value, attr->value_size),
                    QTableWidgetItem::Type);
        attribute_table->setItem(i, 0, key_item);
        attribute_table->setItem(i, 1, val_item);
    }

    // payload text
    ignore_next_payload_text_changed = true;
    payload_text_edit->setPlainText(
            QByteArray(reinterpret_cast<char*>(current_chunk->payload),
                       current_chunk->payload_size));

    ignore_changes = false;
}/*}}}*/

void EditWindow::on_id_line_edit_textChanged ()/*{{{*/
{
    if (ignore_changes) {
        return;
    }

    QString str = id_line_edit->text();
    gas_set_id(current_chunk, qPrintable(str), str.size());
    QtConcurrent::run(gas_update, tree_model->root);
}/*}}}*/

void EditWindow::on_attribute_table_cellChanged (int row, int col)/*{{{*/
{
    if (ignore_changes) {
        return;
    }

    GASattribute* attr = &current_chunk->attributes[row];
    QTableWidgetItem* key_item   = attribute_table->item(row, 0);
    QTableWidgetItem* value_item = attribute_table->item(row, 1);

    switch (col)
    {
    case 0:
    {
        QString key = key_item->text();
        attr->key_size = key.size();
        attr->key = (GASubyte*)realloc(attr->key, attr->key_size + 1);
        memcpy(attr->key, qPrintable(key), attr->key_size);
        attr->key[attr->key_size] = 0;
        break;
    }
    case 1:
    {
        QString value = value_item->text();
        attr->value_size = value.size();
        attr->value = (GASubyte*)realloc(attr->value, attr->value_size + 1);
        memcpy(attr->value, qPrintable(value), attr->value_size);
        attr->value[attr->value_size] = 0;
        break;
    }
    default:
        break;
    }

    QtConcurrent::run(gas_update, tree_model->root);

}/*}}}*/

void EditWindow::on_payload_text_edit_textChanged ()/*{{{*/
{
    if (ignore_next_payload_text_changed) {
        ignore_next_payload_text_changed = false;
        return;
    }
    payload_timer.start(10000);
}/*}}}*/

void EditWindow::on_payload_timer_timeout ()/*{{{*/
{
    qDebug() << "save payload";
    QString str = payload_text_edit->toPlainText();
    gas_set_payload(current_chunk, qPrintable(str), str.size());
    QtConcurrent::run(gas_update, tree_model->root);
}/*}}}*/

// vim: sw=4 fdm=marker
