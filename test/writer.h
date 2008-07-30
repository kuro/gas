
/**
 * @file writer.h
 * @brief writer definition
 */

#pragma once

#include  <QObject>
#include <gas/writer.h>
#include <gas/ntstring.h>

class TestWriter : public QObject
{
    Q_OBJECT

private:
    GASubyte buf[1024];

private slots:
    void test001 (void);
    void test002 (void);
};

// vim: sw=4 fdm=marker
