//
// Created by cdcq on 2021/11/28.
//

#pragma once


#include "layers/disk/interface.h"
#include "layers/disk/logic.h"
#include "layers/disk/guid.h"
#include "layers/image/image.h"

class DiskFactor {
public:
    static DiskReader *createDiskReader(ImageReader *image);

    static DiskReader *createLogicDisk(ImageReader *image);
};

