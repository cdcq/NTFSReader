//
// Created by cdcq on 2021/11/29.
//

#pragma once


#include <QByteArray>

#include "interface.h"

struct NTFSAttribute {
    // TODO: optimize, such as qint64 -> bool.
    qint64 start;
    qint64 type;
    qint64 resident;
    qint64 length;
    qint64 body_offset; // body offset or run list offset.
    qint64 body_length;
};

struct NTFSDataRun {
    qint64 start_cluster;
    qint64 cluster_count;
};

class NTFS : public FilesystemReader {
private:
    const int SECTOR_SIZE_OFFSET_ = 0xb;
    const int SECTOR_SIZE_LENGTH_ = 2;
    const int CLUSTER_SECTORS_COUNT_OFFSET_ = 0xd;
    const int CLUSTER_SECTORS_COUNT_LENGTH_ = 1;
    const int MFT_CLUSTER_OFFSET_ = 0x30;
    const int MFT_CLUSTER_LENGTH_ = 8;
    const int MFT_ENTRY_SIZE_ = 1024;
    const int ROOT_MFT_ID_ = 5;
    const int FILE_ID_OFFSET_ = 0x10;
    const int FILE_ID_LENGTH_ = 2;
    const int FIRST_ATTRIBUTE_OFFSET_ = 0x14;
    const int FIRST_ATTRIBUTE_LENGTH_ = 2;
    const int FILE_FLAG_OFFSET_ = 0x16;
    const int FILE_FLAG_LENGTH_ = 2;
    const int ATTRIBUTE_TYPE_OFFSET_ = 0;
    const int ATTRIBUTE_TYPE_LENGTH_ = 4;
    const int ATTRIBUTE_SIZE_OFFSET_ = 4;
    const int ATTRIBUTE_SIZE_LENGTH_ = 4;
    const int ATTRIBUTE_RESIDENT_OFFSET_ = 8;
    const int ATTRIBUTE_RESIDENT_LENGTH_ = 1;
    const int ATTRIBUTE_BODY_LENGTH_OFFSET_ = 0x10;
    const int ATTRIBUTE_BODY_LENGTH_LENGTH_ = 4;
    const int ATTRIBUTE_BODY_START_OFFSET_ = 0x14;
    const int ATTRIBUTE_BODY_START_LENGTH_ = 2;
    const int ATTRIBUTE_RUN_LIST_START_OFFSET_ = 0x20;
    const int ATTRIBUTE_RUN_LIST_START_LENGTH_ = 2;
    const int INDEX_ROOT_ENTRY_START_OFFSET_ = 0;
    const int INDEX_ROOT_ENTRY_START_LENGTH_ = 4;
    const int INDEX_ATTRIBUTE_ENTRY_START_OFFSET_ = 0x18;
    const int INDEX_ATTRIBUTE_ENTRY_START_LENGTH_ = 4;
    const int INDEX_ENTRY_MFT_OFFSET_ = 0;
    const int INDEX_ENTRY_MFT_LENGTH_ = 8;
    const int INDEX_ENTRY_SIZE_OFFSET_ = 8;
    const int INDEX_ENTRY_SIZE_LENGTH_ = 2;
    const int INDEX_ENTRY_LABEL_OFFSET_ = 0xc;
    const int INDEX_ENTRY_LABEL_LENGTH_ = 2;
    const int INDEX_FILE_SIZE_OFFSET_ = 0x40;
    const int INDEX_FILE_SIZE_LENGTH_ = 8;
    const int INDEX_ENTRY_NAME_LENGTH_OFFSET_ = 0x50;
    const int INDEX_ENTRY_NAME_LENGTH_LENGTH_ = 1;
    const int INDEX_ENTRY_NAME_OFFSET_ = 0x52;
    const int INDEX_ROOT_LENGTH_ = 0x10;
    const int A30H_NAME_LENGTH_OFFSET_ = 0x40;
    const int A30H_NAME_LENGTH_LENGTH_ = 1;
    const int A30H_NAME_OFFSET_ = 0x42;
    const int A80H_ACTUAL_SIZE_OFFSET_ = 0x30;
    const int A80H_ACTUAL_SIZE_LENGTH_ = 8;

    const qint64 FILENAME_TYPE_ = 0x30;
    const qint64 FILE_CONTENT_TYPE_ = 0x80;
    const qint64 INDEX_ROOT_TYPE_ = 0x90;
    const qint64 INDEX_DISTRIBUTE_TYPE_ = 0xa0;
    const qint64 END_TYPE_ = -1;

    qint64 sectorSize_;
    qint64 clusterSectorsCount_;
    qint64 clusterSize_;
    qint64 mftOffset_;
    qint64 rootOffset_;

    void load_();

    NTFSAttribute parseAttributeHead_(qint64 offset);

    QList<FileInfo> parseIndex_(qint64 offset, int type);

    QString parseFileName_(qint64 offset);

    qint64 parseFileSize_(qint64 offset);

    QList<NTFSDataRun> parseRunList_(NTFSAttribute attribute);

    static qint64 findFileInSubFiles_(const QList<FileInfo> &sub_files, const QString& file_name);

public:
    NTFS(int partition_id, DiskReader *disk);

    static bool checkNTFS(int partition_id, DiskReader *disk);

    bool checkFolder(qint64 mft_id) override;

    qint64 locatePath(QString path) override;

    FileInfo readFileInfo(qint64 mft_id) override;

    FileInfo readFileInfo(QString path) override;

    QList<FileInfo> listSubFiles(qint64 mft_id) override;

    QList<FileInfo> listSubFiles(QString path) override;

    int exportFile(qint64 mft_id, QString path) override;

    int exportFile(QString path, QString output_path) override;

};

