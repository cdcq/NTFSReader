//
// Created by cdcq on 2021/11/26.
//

#pragma once


#include <QByteArray>
#include <QObject>
#include <QString>

class ImageReader : QObject {
Q_OBJECT
private:
    QString fileName_;
public:
    virtual QByteArray read(qint64 offset, qint64 count) = 0;

    QString fileName() const;

    void setFileName(const QString &file_name);
};

