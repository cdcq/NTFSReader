//
// Created by cdcq on 2021/11/26.
//

#include "interface.h"

QString ImageReader::fileName() const {
    // TODO: optimize.
    return fileName_;
}

void ImageReader::setFileName(const QString &file_name) {
    fileName_ = file_name;
}

