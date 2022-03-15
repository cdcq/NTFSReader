//
// Created by cdcq on 2021/11/26.
//

#pragma once


#include "dd.h"
#include "e01.h"
#include "interface.h"

class ImageFactor {
public:
    static ImageReader *createImageReader(const QString &file_name);
};

