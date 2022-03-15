//
// Created by cdcq on 2022/2/2.
//

#pragma once


#include "interface.h"

class LogicDisk : public DiskReader {
public:
    explicit LogicDisk(ImageReader *image);

    QByteArray read(int partition, qint64 offset, qint64 count) override;
};
