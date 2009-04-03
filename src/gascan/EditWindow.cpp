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
#include <QFileDialog>
#include <QMessageBox>

#include <gas/fsio.h>
#include <gas/bufio.h>

EditWindow::EditWindow () :
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

    tree_model->setRoot(NULL);

    tree_selection_model = new QItemSelectionModel(tree_model, this); 
    connect(tree_selection_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            SLOT(on_tree_selection_model_currentChanged(QModelIndex, QModelIndex)));
    treeView->setSelectionModel(tree_selection_model);

    payload_timer.setInterval(10000);
    payload_timer.setSingleShot(true);
    connect(&payload_timer, SIGNAL(timeout()), SLOT(on_payload_timer_timeout()));

    setInterfaceEnabled(false);

    connect(exit_action, SIGNAL(triggered()), qApp, SLOT(quit()));

    // attempt to load first file found (if any)
    foreach (QString arg, qApp->arguments()) {
        if (QFile::exists(arg)) {
            load(arg);
            break;
        }
    }
}

EditWindow::~EditWindow ()
{
}

void EditWindow::load (QString fname)
{
    qDebug() << "loading" << fname;
    GASresult gr;
    GASchunk* c;
    GASchunk* dummy;

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

    gas_new_named(&dummy, "dummy");
    gas_add_child(dummy, c);
    tree_model->setRoot(dummy);

    QtConcurrent::run(gas_update, c);

    setInterfaceEnabled(true);
}

void EditWindow::setInterfaceEnabled (bool enabled)
{
    save_action->setEnabled(enabled);
    save_as_action->setEnabled(enabled);
    close_action->setEnabled(enabled);

    id_line_edit->setEnabled(enabled);
    attribute_table->setEnabled(enabled);
    payload_text_edit->setEnabled(enabled);
}

void EditWindow::on_tree_selection_model_currentChanged (
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
}

void EditWindow::on_id_line_edit_editingFinished ()
{
    if (ignore_changes) {
        return;
    }

    QString str = id_line_edit->text();
    gas_set_id(current_chunk, qPrintable(str), str.size());
    QtConcurrent::run(gas_update, tree_model->root);
}

void EditWindow::on_attribute_table_cellChanged (int row, int col)
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
        attr->key = (GASubyte*)gas_realloc(attr->key, attr->key_size + 1, NULL);
        memcpy(attr->key, qPrintable(key), attr->key_size);
        attr->key[attr->key_size] = 0;
        break;
    }
    case 1:
    {
        QString value = value_item->text();
        attr->value_size = value.size();
        attr->value = (GASubyte*)gas_realloc(attr->value, attr->value_size + 1, NULL);
        memcpy(attr->value, qPrintable(value), attr->value_size);
        attr->value[attr->value_size] = 0;
        break;
    }
    default:
        break;
    }

    QtConcurrent::run(gas_update, tree_model->root);

}

void EditWindow::on_payload_text_edit_textChanged ()
{
    if (ignore_next_payload_text_changed) {
        ignore_next_payload_text_changed = false;
        return;
    }
    payload_timer.start(10000);
}

void EditWindow::on_payload_timer_timeout ()
{
    qDebug() << "save payload";
    if (current_chunk == NULL) {
        qWarning() << "no current chunk";
        return;
    }
    QString str = payload_text_edit->toPlainText();
    gas_set_payload(current_chunk, qPrintable(str), str.size());
    QtConcurrent::run(gas_update, tree_model->root);
}

void EditWindow::on_open_action_activated ()
{
    QString fname = QFileDialog::getOpenFileName(this);
    if (fname.isEmpty()) {
        return;
    }
    load(fname);
}

void EditWindow::on_close_action_activated ()
{
    setInterfaceEnabled(false);

    id_line_edit->clear();
    payload_text_edit->clear();
    attribute_table->setRowCount(0);

    payload_timer.stop();  // activated by clearing

    current_chunk = NULL;
    GASchunk* dummy = tree_model->root;
    tree_model->setRoot(NULL);
    gas_destroy(dummy);
}

void EditWindow::on_save_action_activated ()
{
    GASchunk* dummy = tree_model->root;
    if (dummy == NULL) {
        qDebug() << "nothing loaded";
        return;
    }
    for (GASunum i = 0; i < dummy->nb_children; i++) {
        gas_print(dummy->children[i]);
    }
}

void EditWindow::on_save_as_action_activated ()
{
    QString fname = QFileDialog::getSaveFileName(this);
    if (fname.isEmpty()) {
        return;
    }
    qDebug() << fname;
}

void EditWindow::on_about_action_activated ()
{
    QString text =
        "Gas Editor<br />"
        "<br />"
        "See <a href=\"http://github.com/kuro/gas\">http://github.com/kuro/gas</a>"
        ;
    QMessageBox::about(this, "About Gas Editor", text);
}

// vim: sw=4 fdm=syntax
