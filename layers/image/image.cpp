//
// Created by cdcq on 2021/11/26.
//

#include "image.h"

ImageReader* ImageFactor::createImageReader(const QString &file_name) {
    if (E01Image::checkE01File(file_name)) {
        return new E01Image(file_name);
    } else {
        // TODO: check dd image.
        return new DDImage(file_name);
    }
}
