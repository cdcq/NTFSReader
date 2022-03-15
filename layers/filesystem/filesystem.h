//
// Created by cdcq on 2021/12/1.
//

#pragma once


#include "interface.h"
#include "ntfs.h"
#include "layers/disk/disk.h"

class FilesystemFactor {
public:
    static FilesystemReader *createFilesystemReader(int partition_id, DiskReader *disk_reader);
};


