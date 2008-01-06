
/**
 * @file qtedit.h
 * @brief qtedit definition
 */

#pragma once
#include <QMainWindow>
#include <QtGui>


class MyTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    MyTreeModel ();
    ~MyTreeModel ();

    QVariant data (const QModelIndex &index, int role) const;
    Qt::ItemFlags flags (const QModelIndex &index) const;
    QVariant headerData (int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;
    QModelIndex index (int row, int column,
                       const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent (const QModelIndex &index) const;
    int rowCount (const QModelIndex &parent = QModelIndex()) const;
    int columnCount (const QModelIndex &parent = QModelIndex()) const;

    void clear ()
    {
        reset();
    }
};



class MainEditWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainEditWindow ();
    void load (const QString& src);

private slots:
    void on_open (void);
    void on_close (void);
    void on_about (void);
    void on_tree_selection_change (const QModelIndex& current,
                                   const QModelIndex& previous);
    void on_print (void);

private:
    void create_actions (void);
    void create_menus (void);
    void create_toolbars (void);

    QAction *open_action;
    QAction *close_action;
    QAction *exit_action;
    QAction *about_action;
    QAction *print_action;

    QMenu *file_menu;
    QMenu *help_menu;

    QToolBar *file_tool_bar;
    QProgressBar *progress_bar;

    MyTreeModel *model;
    QTreeView *tree_view;
    QItemSelectionModel *tree_selection_model;

    QSplitter *splitter0;
    QSplitter *splitter1;
    QTableWidget *attr_table;
    QTextEdit *payload_box;
};

// vim: sw=4 fdm=marker
