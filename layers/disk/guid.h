//
// Created by cdcq on 2021/11/28.
//

#pragma once


#include "interface.h"

class GuidDisk : public DiskReader {
private:
    const qint64 LBA_SIZE_ = 512;
    const int MBR_OFFSET_ = 0x1BE;
    const int MBR_LENGTH_ = 16;
    const int MBR_FILESYSTEM_OFFSET_ = 4;
    const int MBR_FILESYSTEM_LENGTH_ = 1;
    const int MBR_START_OFFSET_ = 8;
    const int MBR_START_LENGTH_ = 4;
    const int MBR_SIZE_OFFSET_ = 0x0c;
    const int MBR_SIZE_LENGTH_ = 4;
    const int GPT_ENTRY_START_OFFSET_ = 72;
    const int GPT_ENTRY_START_LENGTH_ = 8;
    const int GPT_ENTRY_COUNT_OFFSET_ = 80;
    const int GPT_ENTRY_COUNT_LENGTH_ = 4;
    const int GPT_ENTRY_SIZE_OFFSET_ = 84;
    const int GPT_ENTRY_SIZE_LENGTH_ = 4;
    const int GPT_PARTITION_START_OFFSET_ = 32;
    const int GPT_PARTITION_START_LENGTH_ = 8;
    const int GPT_PARTITION_END_OFFSET_ = 40;
    const int GPT_PARTITION_END_LENGTH_ = 8;
    const int GPT_PARTITION_NAME_OFFSET_ = 56;
    const int GPT_PARTITION_NAME_LENGTH_ = 72;

    void load_();

    void parseGPT_(qint64 offset);

public:
    explicit GuidDisk(ImageReader *image);

    QByteArray read(int partition, qint64 offset, qint64 count) override;
};


