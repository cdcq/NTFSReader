//
// Created by cdcq on 2021/12/1.
//

#include "filesystem.h"

FilesystemReader *FilesystemFactor::createFilesystemReader(int partition_id, DiskReader *disk_reader) {
    if (NTFS::checkNTFS(partition_id, disk_reader)) {
        return new NTFS(partition_id, disk_reader);
    } else {
        return nullptr;
    }
}
