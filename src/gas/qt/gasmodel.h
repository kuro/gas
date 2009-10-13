
/**
 * @file gasmodel.h
 * @brief gasmodel definition
 */

#pragma once

#include <QAbstractItemModel>

namespace Gas
{
namespace Qt
{
class Chunk;
}
}

class GasModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    GasModel (Gas::Qt::Chunk* root, QObject* parent);
    virtual ~GasModel ();

    virtual QModelIndex index(int, int, const QModelIndex&) const;
    virtual QModelIndex parent(const QModelIndex&) const;
    virtual int rowCount(const QModelIndex&) const;
    virtual int columnCount(const QModelIndex&) const;
    virtual QVariant data(const QModelIndex&, int) const;
    
private:
    struct Private;
    Private* d;
};
