//
// Created by cdcq on 2021/11/28.
//

#include "guid.h"

#include "qdebug.h"

void GuidDisk::load_() {
    if (image() == nullptr) {
        // TODO: warning.
        // TODO: check GUID disk.
        return;
    }
    // TODO: MBR/GPT mix type disk.
    for (int i = 0; i < 4; i++) {
        qint64 offset = MBR_OFFSET_ + i * MBR_LENGTH_;
        QByteArray filesystem_label = image()->read(
                offset + MBR_FILESYSTEM_OFFSET_, MBR_FILESYSTEM_LENGTH_);
        // TODO: partition type.
        if (filesystem_label == QByteArray("\xEE")) {
            QByteArray offset_data = image()->read(
                    offset + MBR_START_OFFSET_, MBR_START_LENGTH_);
            parseGPT_(*(qint32 *) offset_data.constData() * LBA_SIZE_);
        } else if (filesystem_label == QByteArray("\x07")) {
            PartitionInfo partition{};
            QByteArray start_data = image()->read(
                    offset + MBR_START_OFFSET_, MBR_START_LENGTH_);
            // TODO: check for the use of int32 on address.
            partition.start = *(qint32 *) start_data.constData() * LBA_SIZE_;
            QByteArray size_data = image()->read(offset + MBR_SIZE_OFFSET_, MBR_SIZE_LENGTH_);
            partition.size = *(qint32 *) size_data.constData() * LBA_SIZE_;
            partition.name = QString("MBR partition ").append(QString::number(i));
            appendPartition(partition);
            continue;
        }
    }
}

void GuidDisk::parseGPT_(qint64 offset) {
    QByteArray start_data = image()->read(
            offset + GPT_ENTRY_START_OFFSET_, GPT_ENTRY_START_LENGTH_);
    qint64 entry_start = *(qint64 *) start_data.constData() * LBA_SIZE_;
    if (entry_start == 0) {
        return;
    }
    QByteArray count_data = image()->read(
            offset + GPT_ENTRY_COUNT_OFFSET_, GPT_ENTRY_COUNT_LENGTH_);
    qint64 entry_count = *(qint32 *) count_data.constData();
    QByteArray size_data = image()->read(
            offset + GPT_ENTRY_SIZE_OFFSET_, GPT_ENTRY_SIZE_LENGTH_);
    qint64 entry_size = *(qint32 *) size_data.constData();
    for (int i = 0; i < entry_count; i++) {
        qint64 entry_offset = entry_start + i * entry_size;
        QByteArray partition_start_data = image()->read(
                entry_offset + GPT_PARTITION_START_OFFSET_, GPT_PARTITION_START_LENGTH_);
        qint64 partition_start = *(qint64 *) partition_start_data.constData() * LBA_SIZE_;
        if (partition_start == 0) {
            continue;
        }
        QByteArray partition_end_data = image()->read(
                entry_offset + GPT_PARTITION_END_OFFSET_, GPT_PARTITION_END_LENGTH_);
        qint64 partition_end = *(qint64 *) partition_end_data.constData() * LBA_SIZE_;
        qint64 partition_size = partition_end - partition_start;
        QByteArray partition_name_data = image()->read(
                entry_offset + GPT_PARTITION_NAME_OFFSET_, GPT_PARTITION_NAME_LENGTH_);
        partition_name_data.append(QByteArrayLiteral("\x00\x00"));
        QString partition_name = QString::fromUtf16((const ushort *) partition_name_data.data());
        PartitionInfo partition{partition_start, partition_size, partition_name};
        appendPartition(partition);
    }
}

GuidDisk::GuidDisk(ImageReader *image) {
    setImage(image);

    load_();
}

QByteArray GuidDisk::read(int partition_id, qint64 offset, qint64 count) {
    if (partition_id > partitions().size()) {
        // TODO: warning.
        return {""};
    }
    PartitionInfo partition = partitions()[partition_id];
    if (partition.size - offset <= count) {
        // TODO: warning.
        return {""};
    }
    return image()->read(partition.start + offset, count);
}
