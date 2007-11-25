/**
 * @file qtedit.cpp
 * @brief qtedit implementation
 */

#include <stdio.h>

#include <QApplication>
#include <QtGui>
#include <QtGui>

#include "qtedit.h"
#include "qtedit.moc"

#include <gas/parser.h>
#include <gas/ntstring.h>

static gas_context *ctx;
static gas_parser *parser;
static chunk *root;

/* MainEditWindow {{{*/
/* cons {{{*/
MainEditWindow::MainEditWindow (void)
{
    create_actions();
    create_menus();
    create_toolbars();
    statusBar()->showMessage(tr("Ready"));

    progress_bar = new QProgressBar(this);
    statusBar()->addPermanentWidget(progress_bar);


    model = new MyTreeModel();
    tree_view = new QTreeView(this);
    tree_view->setModel(model);

    tree_selection_model = new QItemSelectionModel(model, this); 
    connect(tree_selection_model, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
            this, SLOT(on_tree_row_change(QModelIndex, QModelIndex)));
    tree_view->setSelectionModel(tree_selection_model);

    attr_table = new QTableWidget(this);
    attr_table->setColumnCount(2);
    QStringList attr_table_headers;
    attr_table_headers << "Key" << "Value";
    attr_table->setHorizontalHeaderLabels(attr_table_headers);
    //attr_table->setReadOnly(true);
    attr_table->setSortingEnabled(true);

    payload_box = new QTextEdit(this);
    payload_box->setReadOnly(true);

    splitter0 = new QSplitter(this);
    splitter1 = new QSplitter(Qt::Vertical, this);

    //new QLabel("Attributes:", attr_table);
    //splitter1->addWidget(new QLabel("Attributes:"));
    splitter1->addWidget(attr_table);
    splitter1->addWidget(new QLabel("Payload:"));
    splitter1->addWidget(payload_box);
    splitter0->addWidget(tree_view);
    splitter0->addWidget(splitter1);

    setCentralWidget(splitter0);
}
/*}}}*/
/* creation {{{*/
/* create_actions {{{*/
void MainEditWindow::create_actions (void)
{
    open_action = new QAction(tr("&Open"), this);
    open_action->setShortcut(tr("Ctrl+O"));
    open_action->setStatusTip(tr("Open a file"));
    connect(open_action, SIGNAL(triggered()), this, SLOT(on_open()));

    close_action = new QAction(tr("&Close"), this);
    close_action->setShortcut(tr("Ctrl+W"));
    close_action->setStatusTip(tr("Close file"));
    close_action->setEnabled(false);
    connect(close_action, SIGNAL(triggered()), this, SLOT(on_close()));

    print_action = new QAction(tr("&Print"), this);
    print_action->setShortcut(tr("Ctrl+P"));
    print_action->setStatusTip(tr("Print"));
    print_action->setEnabled(false);
    connect(print_action, SIGNAL(triggered()), this, SLOT(on_print()));

    exit_action = new QAction(tr("E&xit"), this);
    exit_action = new QAction(tr("E&xit"), this);
    exit_action->setShortcut(tr("Ctrl+Q"));
    exit_action->setStatusTip(tr("Exit the application"));
    connect(exit_action, SIGNAL(triggered()), this, SLOT(close()));

    about_action = new QAction(tr("&About"), this);
    about_action->setStatusTip(tr("Display information about application"));
    connect(about_action, SIGNAL(triggered()), this, SLOT(on_about()));
}
/*}}}*/
/* create_menus {{{*/
void MainEditWindow::create_menus (void)
{
    file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(open_action);
    file_menu->addAction(close_action);
    file_menu->addSeparator();
    file_menu->addAction(print_action);
    file_menu->addSeparator();
    file_menu->addAction(exit_action);

    menuBar()->addSeparator();
    help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(about_action);
}
/*}}}*/
/* create_toolbars {{{*/
void MainEditWindow::create_toolbars (void)
{
    file_tool_bar = addToolBar(tr("File"));
    file_tool_bar->addAction(open_action);
    file_tool_bar->addAction(print_action);

    file_tool_bar->addSeparator();
    file_tool_bar->addAction(close_action);
}
/*}}}*/
/*}}}*/
/* slots {{{*/
void MainEditWindow::on_open (void)
{
    QString fname = QFileDialog::getOpenFileName(this);
    if (fname.isEmpty()) {
        return;
    }

    load(fname);
}

void MainEditWindow::on_close (void)
{
    close_action->setEnabled(false);
    print_action->setEnabled(false);

    if (root) {
        gas_destroy(root);
        root = NULL;
    }

    //model->modelReset();

    setWindowTitle("gascan");
}

void MainEditWindow::on_print (void)
{
    // gas_print will not die if root is NULL
    gas_print(root);

#if 0
    QPrinter *printer = new QPrinter;
    QPrintDialog diag (printer, this);
    if (diag.exec() == QDialog::Accepted) {
        QPainter p (printer);
        p.drawText(0, 0, "Hello World");
    }
    delete printer;
#endif
}

void MainEditWindow::on_about (void)
{
    //qApp->aboutQt();
    QMessageBox::about(this, tr("About Application"), tr("Gas Editor"));
}

void MainEditWindow::on_tree_row_change (const QModelIndex& current,
        const QModelIndex& previous)
{
    chunk *c = static_cast<chunk*>(current.internalPointer());

    attr_table->clearContents();
    attr_table->setRowCount(c->nb_attributes);

    for (GASunum i = 0; i < c->nb_attributes; i++) {
        QByteArray key ((char*)c->attributes[i].key,
                c->attributes[i].key_size);
        QTableWidgetItem *key_item = new QTableWidgetItem (QString(key));
        key_item->setToolTip(QString::number(c->attributes[i].key_size));
        attr_table->setItem(i, 0, key_item);

        QByteArray val ((char*)c->attributes[i].value,
                c->attributes[i].value_size);
        QTableWidgetItem *val_item = new QTableWidgetItem (QString(val));
        val_item->setToolTip(QString::number(c->attributes[i].value_size));
        attr_table->setItem(i, 1, val_item);
    }

    QByteArray payload ((char*)c->payload, c->payload_size);
    payload_box->setText(payload);
}
/*}}}*/

void MainEditWindow::load (const QString& src)
{
    progress_bar->show();
    progress_bar->setMaximum(0);
    //progress_bar->setValue(50);

    close_action->setEnabled(true);
    print_action->setEnabled(true);

    QFileInfo finfo (src);
    QString title = finfo.fileName() + " (" + finfo.absolutePath() + ") - gascan";
    setWindowTitle(title);

    if (root) {
        gas_destroy(root);
        root = NULL;
    }

    root = gas_new(4, "root");
    chunk *doc = gas_parse(parser, src.toAscii());
    gas_add_child(root, doc);

    progress_bar->hide();
}

/*}}}*/
/* MyTreeModel {{{*/
/* MyTreeModel() {{{*/
MyTreeModel::MyTreeModel ()
{
}
/*}}}*/
/* ~MyTreeModel() {{{*/
MyTreeModel::~MyTreeModel ()
{
}
/*}}}*/
/* columnCount() {{{*/
int MyTreeModel::columnCount (const QModelIndex &parent) const
{
    //qDebug("column count");
    return 2;
}
/*}}}*/
/* data() {{{*/
QVariant MyTreeModel::data (const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }


    chunk *c = static_cast<chunk*>(index.internalPointer());
    if (index.column() == 0) {
        qDebug("\t\tdata row=%d id=%s", index.row(), (char*)c->id);
        return QByteArray((char*)c->id, c->id_size);
    } else {
        return (unsigned int)c->size;
    }

}
/*}}}*/
/* flags() {{{*/
Qt::ItemFlags MyTreeModel::flags (const QModelIndex &index) const
{
    //qDebug("flags");
    if (!index.isValid()) {
        return 0;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
/*}}}*/
/* headerData() {{{*/
QVariant MyTreeModel::headerData (int section, Qt::Orientation orientation,
        int role) const
{
    //qDebug("header section=%d role=%d", section, role);
    if (role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return "Chunk";
        case 1:
            return "Size";
        default:
            //qDebug("unprovided header");
            break;
        }
    }
    //return QString("header");
    return QVariant();
}
/*}}}*/
/* index() {{{*/
QModelIndex MyTreeModel::index (int row, int col,
        const QModelIndex &parent) const
{
    if (root == NULL) {
        return QModelIndex();
    }

    qDebug("index row=%d col=%d", row, col);

    if ( ! hasIndex(row, col, parent)) {
        return QModelIndex();
    }

    chunk *parent_chunk;
    if (parent.isValid()) {
        parent_chunk = static_cast<chunk*>(parent.internalPointer());
    } else {
        parent_chunk = root;
    }

    qDebug("parent was %s\n", (char*)parent_chunk->id);

    if ((GASunum)row < parent_chunk->nb_children) {
//        qDebug("      row=%d col=%d is id=%s", row, col,
//                (char*)parent_chunk->children[row]->id);
        return createIndex(row, col, parent_chunk->children[row]);
    } else {
        return QModelIndex();
    }
}
/*}}}*/
/* parent() {{{*/
QModelIndex MyTreeModel::parent (const QModelIndex &index) const
{


    if ( ! index.isValid()) {
        return QModelIndex();
    }
   
    chunk *current_chunk;
    chunk *parent_chunk;

    current_chunk = static_cast<chunk*>(index.internalPointer());
    qDebug("parent for %s", (char*)current_chunk->id);
    parent_chunk = current_chunk->parent;

    if (parent_chunk == NULL) {
        return QModelIndex();
    }

    // determine the parent's row
    int row;
    if (parent_chunk->parent == NULL) {
        row = 0;
    } else {
        qDebug("%ld", parent_chunk->parent->nb_children);
        for (unsigned int i = 0; i < parent_chunk->parent->nb_children; i++) {
            if (parent_chunk->parent->children[i] == parent_chunk) {
                row = i;
                break;
            }
        }
    }
qDebug("created index at row %d for %s", row, (char*)parent_chunk->id);
    return createIndex(row, 0, parent_chunk);
}
/*}}}*/
/* rowCount() {{{*/
int MyTreeModel::rowCount (const QModelIndex &parent) const
{
    if (root == NULL) {
        return 0;
    }
    if (parent.column() > 0) {
        return 0;
    }

    chunk *parent_chunk;
    if ( ! parent.isValid()) {
        parent_chunk = root;
    } else {
        parent_chunk = static_cast<chunk*>(parent.internalPointer());
    }

    return parent_chunk->nb_children;
}
/*}}}*/
/*}}}*/
/* main() {{{*/
int qtedit_main (int argc, char **argv)
{
    int retval;

    ctx = gas_context_new();
    parser = gas_parser_new(ctx);
    root = NULL;

    QApplication app (argc, argv);
    MainEditWindow win;
    //win.load("test/dump.gas");
    win.load("test.gas");
    win.show();

    retval = app.exec();

    if (root) {
        gas_destroy(root);
        root = NULL;
    }

    return retval;
}
/*}}}*/
// vim: sw=4 fdm=marker
