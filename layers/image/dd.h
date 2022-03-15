//
// Created by cdcq on 2021/11/27.
//

#pragma once


#include <QByteArray>
#include <QFile>
#include <QString>

#include "interface.h"

class DDImage : public ImageReader {
public:
    explicit DDImage(const QString &file_name);

    QByteArray read(qint64 offset, qint64 count) override;
};

