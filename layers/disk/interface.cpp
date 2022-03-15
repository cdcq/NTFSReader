//
// Created by cdcq on 2021/11/28.
//

#include "interface.h"

ImageReader *DiskReader::image() {
    return image_;
}

void DiskReader::setImage(ImageReader *image) {
    image_ = image;
}

QList<PartitionInfo> DiskReader::partitions() {
    // TODO: optimize.
    return partitions_;
}

void DiskReader::appendPartition(const PartitionInfo &partition) {
    partitions_.append(partition);
}
