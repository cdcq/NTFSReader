//
// Created by cdcq on 2021/11/28.
//

#pragma once


#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>

#include "layers/image/interface.h"

struct PartitionInfo {
    qint64 start;
    qint64 size;
    QString name;
    // TODO: properties label;
};

class DiskReader : public QObject {
Q_OBJECT
private:
    ImageReader *image_;
    QList<PartitionInfo> partitions_;
public:
    virtual QByteArray read(int partition, qint64 offset, qint64 count) = 0;

    ImageReader *image();

    void setImage(ImageReader *image);

    QList<PartitionInfo> partitions();

    void appendPartition(const PartitionInfo &partition);
};

