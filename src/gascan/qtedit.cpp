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
    tree_view->setEnabled(false);
    tree_view->setModel(model);
    tree_view->setAlternatingRowColors(true);

    tree_selection_model = new QItemSelectionModel(model, this); 
    connect(tree_selection_model, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(on_tree_selection_change(QModelIndex, QModelIndex)));
    tree_view->setSelectionModel(tree_selection_model);

    attr_table = new QTableWidget(this);
    attr_table->setEnabled(false);
    attr_table->setAlternatingRowColors(true);
    attr_table->setColumnCount(2);
    QStringList attr_table_headers;
    attr_table_headers << "Key" << "Value";
    attr_table->setHorizontalHeaderLabels(attr_table_headers);
    //attr_table->setReadOnly(true);
    attr_table->setSortingEnabled(true);

    payload_box = new QTextEdit(this);
    payload_box->setEnabled(false);
    payload_box->setReadOnly(true);

    splitter0 = new QSplitter(this);
    splitter1 = new QSplitter(Qt::Vertical, this);

    //new QLabel("Attributes:", attr_table);
    //splitter1->addWidget(new QLabel("Attributes:"));
    splitter1->addWidget(attr_table);
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

    attr_table->setEnabled(false);
    tree_view->setEnabled(false);
    payload_box->setEnabled(false);

    if (root) {
        gas_destroy(root);
        root = NULL;
    }

    model->clear();
    attr_table->clearContents();
    payload_box->clear();

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

QString sanitize (const QByteArray& in, bool wrap)
{
    char buf[8];
    QString out;
    bool doit = false;
    for (int i = 0; i < in.size(); i++) {
        if (in[i] == 0 or not isprint(in[i])) {
            doit = true;
            break;
        }
    }
    if (doit) {
        if (wrap) {
            for (int i = 0; i < in.size(); i++) {
                snprintf(buf, sizeof(buf), "<%02x>", in[i] & 0xff);
                out.append(buf);
                if (i % 8 == 7) {
                    out.append('\n');
                }
            }
        } else {
            for (int i = 0; i < in.size(); i++) {
                snprintf(buf, sizeof(buf), "<%02x>", in[i] & 0xff);
                out.append(buf);
            }
        }
        return out;
    } else {
        return in;
    }
}

void MainEditWindow::on_tree_selection_change (const QModelIndex& current,
                                               const QModelIndex& previous)
{
    chunk *c = static_cast<chunk*>(current.internalPointer());

    attr_table->clearContents();
    attr_table->setRowCount(c->nb_attributes);

    for (GASunum i = 0; i < c->nb_attributes; i++) {
        QByteArray key ((char*)c->attributes[i].key,
                c->attributes[i].key_size);
        QTableWidgetItem *key_item = new QTableWidgetItem (sanitize(key, false));
        key_item->setToolTip(QString::number(c->attributes[i].key_size));
        attr_table->setItem(i, 0, key_item);

        QByteArray val ((char*)c->attributes[i].value,
                c->attributes[i].value_size);
        QTableWidgetItem *val_item = new QTableWidgetItem (sanitize(val, false));
        val_item->setToolTip(QString::number(c->attributes[i].value_size));
        attr_table->setItem(i, 1, val_item);
    }

    QByteArray payload ((char*)c->payload, c->payload_size);
    //payload_box->setText(payload);
    payload_box->setText(sanitize(payload, true));
}
/*}}}*/
// load {{{
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

    root = gas_new("root", 4);
    chunk *doc = gas_parse(parser, src.toAscii());
    gas_add_child(root, doc);

    progress_bar->hide();

    attr_table->setEnabled(true);
    tree_view->setEnabled(true);
    payload_box->setEnabled(true);

    model->clear();
}
// }}}
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
    return 3;
}
/*}}}*/
/* flags() {{{*/
Qt::ItemFlags MyTreeModel::flags (const QModelIndex &index) const
{
    //qDebug("flags");
    if (not index.isValid()) {
        return Qt::ItemIsEnabled;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
/*}}}*/
/* headerData() {{{*/
QVariant MyTreeModel::headerData (int section, Qt::Orientation orientation,
        int role) const
{
    //qDebug("header section=%d role=%d", section, role);
    if (orientation == Qt::Horizontal and role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Chunk");
        case 1:
            return tr("Attributes");
        case 2:
            return tr("Size");
        default:
            //qDebug("unprovided header");
            return QVariant();
        }
    }
    //return QString("header");
    return QVariant();
}
/*}}}*/

/* data() {{{*/
QVariant MyTreeModel::data (const QModelIndex &index, int role) const
{
    if (not index.isValid()) {
        return QVariant();
    }

    chunk *c = static_cast<chunk*>(index.internalPointer());

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
int MyTreeModel::rowCount (const QModelIndex &parent) const
{
    chunk *parent_chunk;

    if (root == NULL) {
        return 0;
    }

//    if (parent.column() > 0) {
//        return 0;
//    }

    if (not parent.isValid()) {
        parent_chunk = root;
    } else {
        parent_chunk = static_cast<chunk*>(parent.internalPointer());
    }

    return parent_chunk->nb_children;
}
/*}}}*/
/* index() {{{*/
QModelIndex MyTreeModel::index (int row, int col,
        const QModelIndex &parent) const
{
    chunk *parent_chunk;

//    if (root == NULL) {
//        return QModelIndex();
//    }

//    if ( ! hasIndex(row, col, parent)) {
//        return QModelIndex();
//    }

    if (not parent.isValid()) {
        parent_chunk = root;
    } else {
        parent_chunk = static_cast<chunk*>(parent.internalPointer());
    }

    //qDebug("parent was %s\n", (char*)parent_chunk->id);

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
QModelIndex MyTreeModel::parent (const QModelIndex &child) const
{
    if (not child.isValid()) {
        return QModelIndex();
    }
   
    chunk *child_chunk = static_cast<chunk*>(child.internalPointer());
    chunk *parent_chunk = child_chunk->parent;;

    //qDebug("parent for %s", (char*)child_chunk->id);

    if (parent_chunk == NULL or parent_chunk == root) {
        return QModelIndex();
    }

    // determine the parent's row
    int row;
    if (parent_chunk->parent == NULL) {
        row = 0;
    } else {
        //qDebug("%ld", parent_chunk->parent->nb_children);
        for (unsigned int i = 0; i < parent_chunk->parent->nb_children; i++) {
            if (parent_chunk->parent->children[i] == parent_chunk) {
                row = i;
                break;
            }
        }
    }
//qDebug("created child at row %d for %s", row, (char*)parent_chunk->id);
    return createIndex(row, 0, parent_chunk);
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

    if (argc > 1) {
        win.load(argv[argc-1]);
    }

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
