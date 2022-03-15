//
// Created by cdcq on 2022/2/2.
//

#include "logic.h"

LogicDisk::LogicDisk(ImageReader *image) {
    setImage(image);
}

QByteArray LogicDisk::read(int partition_id, qint64 offset, qint64 count) {
    return image()->read(offset, count);
}
