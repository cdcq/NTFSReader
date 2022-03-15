//
// Created by cdcq on 2021/11/28.
//

#include "disk.h"

DiskReader* DiskFactor::createDiskReader(ImageReader *image) {
    // TODO: check disk type.
    return new GuidDisk(image);
}

DiskReader* DiskFactor::createLogicDisk(ImageReader *image) {
    return new LogicDisk(image);
}
