//
// Created by cdcq on 2021/11/29.
//

#pragma once


#include <QString>

#include "layers/disk/interface.h"

struct FileInfo {
    qint64 id; // mft id or inode.
    qint64 size;
    QString name;
    // TODO: parse file time.
    // qint64 createTime;
    // qint64 modifyTime;
    // qint64 accessTime;
};

class FilesystemReader : QObject {
Q_OBJECT
private:
    DiskReader *disk_;
    int partition_id_;
public:
    virtual bool checkFolder(qint64 mft_id) = 0;

    virtual qint64 locatePath(QString path) = 0;

    virtual FileInfo readFileInfo(qint64 mft_id) = 0;

    virtual FileInfo readFileInfo(QString path) = 0;

    virtual QList<FileInfo> listSubFiles(qint64 mft_id) = 0;

    virtual QList<FileInfo> listSubFiles(QString path) = 0;

    virtual int exportFile(qint64 mft_id, QString path) = 0;

    virtual int exportFile(QString path, QString output_path) = 0;

    QByteArray readInPartition(qint64 offset, qint64 count);

    DiskReader *disk();

    void setDisk(DiskReader *disk);

    int partitionId() const;

    void setPartitionId(int id);
};

