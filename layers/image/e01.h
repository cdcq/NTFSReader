//
// Created by cdcq on 2021/11/26.
//

#pragma once


#include <QByteArray>
#include <QFile>
#include <QList>
#include <QString>

#include "interface.h"

enum E01SectionType {
    Header, Volume, Table, Next, Done,
    Header2, Disk, Sectors, Table2, Data, Errors2, Session, Hash, Digest,
    Unknown,
};

class E01Section {
private:
    const int SECTION_HEAD_SIZE_ = 76;
    const int NEXT_OFFSET_ = 16;
    const int NEXT_LENGTH_ = 8;
    const int SIZE_OFFSET_ = 24;
    const int SIZE_LENGTH_ = 8;
    const int CHECK_SUM_OFFSET_ = 72;
    const int CHECK_SUM_LENGTH_ = 4;

    E01SectionType type_;
    qint64 next_;
    qint64 size_;
    qint64 checksum_;
public:
    E01Section();

    void parseDescriptor(const QByteArray &descriptor);

    E01SectionType type() const;

    qint64 next() const;

    qint64 size() const;
};

struct E01Table {
    qint64 start;  // The start_cluster offset in image.
    qint64 count;  // The count of table entries.
    qint64 size;  // The number of bytes in source disk.
    qint64 offset;  // The start_cluster offset in source disk.
    qint64 base;  // The data chunk offset relate to this base.
};

class E01Image : public ImageReader {
private:
    const int EWF_HEAD_SIZE_ = 13;
    const int SECTION_HEAD_SIZE_ = 76;
    const int TABLE_HEAD_SIZE_ = 24;
    const int ENTRY_COUNT_SIZE_ = 4;
    const int TABLE_ENTRY_SIZE_ = 4;
    const int TABLE_BASE_OFFSET_ = 84;
    const int TABLE_BASE_LENGTH_ = 8;

    qint64 sectorCount_;  // The number of sectors per chunk
    qint64 sectorSize_;  // The number of bytes per sectors.
    qint64 chunkSize_;  // The number of bytes sectors per chunk.
    QList<E01Table> tables_;

    void load_();

    int locateTable_(qint64 offset);

    qint64 locateChunk_(E01Table table, qint64 location);

    QByteArray readInFile_(qint64 offset, qint64 count);

    QByteArray readInTable_(E01Table table, qint64 offset, qint64 count);

public:
    explicit E01Image(const QString &file_name);

    static bool checkE01File(const QString &file_name);

    QByteArray read(qint64 offset, qint64 count) override;
};

