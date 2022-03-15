//
// Created by cdcq on 2021/11/29.
//

#include "interface.h"

QByteArray FilesystemReader::readInPartition(qint64 offset, qint64 count) {
    return disk_->read(partition_id_, offset, count);
}

DiskReader *FilesystemReader::disk() {
    return disk_;
}

void FilesystemReader::setDisk(DiskReader *disk) {
    disk_ = disk;
}

int FilesystemReader::partitionId() const {
    return partition_id_;
}

void FilesystemReader::setPartitionId(int id) {
    partition_id_ = id;
}
