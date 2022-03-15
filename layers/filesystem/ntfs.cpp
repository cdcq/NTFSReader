//
// Created by cdcq on 2021/11/29.
//

#include "ntfs.h"

#include <QDebug>
#include <QDir>

NTFS::NTFS(int partition_id, DiskReader *disk) {
    sectorSize_ = 0x200;
    clusterSectorsCount_ = 8;
    clusterSize_ = sectorSize_ * clusterSectorsCount_;
    mftOffset_ = -1;
    rootOffset_ = -1;
    setPartitionId(partition_id);
    setDisk(disk);

    load_();
}

bool NTFS::checkNTFS(int partition_id, DiskReader *disk) {
    const int NTFS_LABEL_OFFSET_ = 0x3;
    const int NTFS_LABEL_LENGTH_ = 8;
    QByteArray ntfs_label = disk->read(partition_id, NTFS_LABEL_OFFSET_, NTFS_LABEL_LENGTH_);
    if (ntfs_label != QByteArray("NTFS    ")) {
        return false;
    }
    return true;
}

void NTFS::load_() {
    if (!checkNTFS(partitionId(), disk())) {
        // TODO: warning.
        return;
    }
    QByteArray sector_size_data = readInPartition(SECTOR_SIZE_OFFSET_, SECTOR_SIZE_LENGTH_);
    sectorSize_ = *(qint16 *) sector_size_data.constData();
    QByteArray cluster_sectors_count_data = readInPartition(
            CLUSTER_SECTORS_COUNT_OFFSET_, CLUSTER_SECTORS_COUNT_LENGTH_);
    clusterSectorsCount_ = *(unsigned char *) cluster_sectors_count_data.constData();
    clusterSize_ = sectorSize_ * clusterSectorsCount_;
    QByteArray mft_cluster_data = readInPartition(MFT_CLUSTER_OFFSET_, MFT_CLUSTER_LENGTH_);
    qint64 mft_cluster = *(qint32 *) mft_cluster_data.constData();
    mftOffset_ = mft_cluster * clusterSize_;
    rootOffset_ = mftOffset_ + ROOT_MFT_ID_ * MFT_ENTRY_SIZE_;
}

NTFSAttribute NTFS::parseAttributeHead_(qint64 offset) {
    NTFSAttribute attribute{};
    attribute.start = offset;
    QByteArray type_data = readInPartition(
            offset + ATTRIBUTE_TYPE_OFFSET_, ATTRIBUTE_TYPE_LENGTH_);
    attribute.type = *(qint32 *) type_data.constData();
    if (attribute.type == END_TYPE_) {
        return attribute;
    }
    QByteArray length_data = readInPartition(
            offset + ATTRIBUTE_SIZE_OFFSET_, ATTRIBUTE_SIZE_LENGTH_);
    attribute.length = *(qint32 *) length_data.constData();
    QByteArray resident_data = readInPartition(
            offset + ATTRIBUTE_RESIDENT_OFFSET_, ATTRIBUTE_RESIDENT_LENGTH_);
    attribute.resident = *(unsigned char *) resident_data.constData();
    if (attribute.resident == 0) {
        QByteArray offset_data = readInPartition(
                offset + ATTRIBUTE_BODY_START_OFFSET_, ATTRIBUTE_BODY_START_LENGTH_);
        attribute.body_offset = *(qint16 *) offset_data.constData();
        QByteArray body_length_data = readInPartition(
                offset + ATTRIBUTE_BODY_LENGTH_OFFSET_, ATTRIBUTE_BODY_LENGTH_LENGTH_);
        attribute.body_length = *(qint32 *) body_length_data.constData();
    } else {
        QByteArray offset_data = readInPartition(
                offset + ATTRIBUTE_RUN_LIST_START_OFFSET_, ATTRIBUTE_RUN_LIST_START_LENGTH_);
        attribute.body_offset = *(qint16 *) offset_data.constData();
    }
    return attribute;
}

QList<FileInfo> NTFS::parseIndex_(qint64 offset, int type) {
    // TODO: why some files appeared twice?

    // Type = 0 for index root and type = 1 for index attribute.
    if (type && readInPartition(offset, 4) != QByteArray("INDX")) {
        // TODO: warning.
        return {};
    }
    if (!type) {
        offset += INDEX_ROOT_LENGTH_;
        QByteArray start_data = readInPartition(
                offset + INDEX_ROOT_ENTRY_START_OFFSET_, INDEX_ROOT_ENTRY_START_LENGTH_);
        offset += *(qint32 *) start_data.constData();
    } else {
        QByteArray start_data = readInPartition(
                offset + INDEX_ATTRIBUTE_ENTRY_START_OFFSET_, INDEX_ATTRIBUTE_ENTRY_START_LENGTH_);
        offset += *(qint32 *) start_data.constData() + INDEX_ATTRIBUTE_ENTRY_START_OFFSET_;
    }
    QList<FileInfo> ret{};
    QList<qint64> id_set{};
    for (;;) {
        QByteArray mft_data = readInPartition(
                offset + INDEX_ENTRY_MFT_OFFSET_, INDEX_ENTRY_MFT_LENGTH_);
        qint64 mft_id = *(qint64 *) mft_data.constData();
        mft_id &= 0xffffffffffffLL; // get first 6 bytes for mft id, the last 2 bytes if serial number.
        QByteArray size_data = readInPartition(
                offset + INDEX_ENTRY_SIZE_OFFSET_, INDEX_ENTRY_SIZE_LENGTH_);
        qint64 entry_size = *(qint16 *) size_data.constData();
        QByteArray label_data = readInPartition(
                offset + INDEX_ENTRY_LABEL_OFFSET_, INDEX_ENTRY_LABEL_LENGTH_);
        qint64 entry_label = *(qint16*) label_data.constData();
        FileInfo file_info = readFileInfo(mft_id);
        if (id_set.count(mft_id) == 0) {
            id_set.append(mft_id);
            ret.append(file_info);
        }
        if (entry_label & 2) {
            // TODO: find out what happened when label = 1.
            break;
        }
        offset += entry_size;
    }
    return ret;
}

QString NTFS::parseFileName_(qint64 offset) {
    QByteArray name_length_data = readInPartition(offset + A30H_NAME_LENGTH_OFFSET_, A30H_NAME_LENGTH_LENGTH_);
    int name_length = *(unsigned char *) name_length_data.constData();
    name_length *= 2;
    QByteArray name_data = readInPartition(offset + A30H_NAME_OFFSET_, name_length);
    name_data.append(QByteArrayLiteral("\x00\x00"));
    QString file_name = QString::fromUtf16((const ushort *) name_data.data());
    QByteArray temp_data = readInPartition(offset + 0x41, 1);
    int file_name_space = *(unsigned char *)temp_data.constData();
    // TODO: Find out what's the mean of file name space
    if (file_name_space != 2) {
        return file_name;
    } else {
        // Return an empty string when it is a short name.
        return "";
    }
}

qint64 NTFS::parseFileSize_(qint64 offset) {
    QByteArray size_data = readInPartition(offset + A80H_ACTUAL_SIZE_OFFSET_, A80H_ACTUAL_SIZE_LENGTH_);
    qint64 size = *(qint64 *)size_data.constData();
    return size;
}

QList<NTFSDataRun> NTFS::parseRunList_(NTFSAttribute attribute) {
    qint64 offset = attribute.start;
    qint64 end_offset = offset + attribute.length;
    offset += attribute.body_offset;
    qint64 last_cluster = 0;
    QList<NTFSDataRun> run_list{};
    while (offset < end_offset) {
        // What happend when debug??
        QByteArray length_data = readInPartition(offset, 1);
        int length = *(unsigned char *) length_data.constData();
        int low_length = length % 16;
        int high_length = length / 16;
        if (low_length == 0 || high_length == 0) {
            // TODO: check check correctness.
            break;
        }
        QByteArray count_data = readInPartition(offset + 1, low_length + high_length);
        qint64 cluster_count = 0;
        for (int i = low_length - 1; i >= 0; i--) {
            cluster_count *= 256;
            cluster_count += (unsigned char)count_data[i];
        }
        qint64 start_cluster = 0;
        for (int i = high_length - 1; i >= 0; i--) {
            start_cluster *= 256;
            start_cluster += (unsigned char)count_data[low_length + i];
        }
        if (last_cluster && (count_data[low_length + high_length - 1] & 0x80)) {
            // TODO: check correctness.
            // turn signed number to minus number;
            start_cluster &= (qint64)-1;
            start_cluster *= -1;
        }
        // TODO: will an INDEX use two data runs?
        start_cluster += last_cluster;
        last_cluster = start_cluster;
        run_list.append(NTFSDataRun{start_cluster, cluster_count});
        offset += low_length + high_length + 1;
    }
    return run_list;
}

qint64 NTFS::findFileInSubFiles_(const QList<FileInfo> &sub_files, const QString &file_name) {
    // If not found, return -1.
    for (auto &i: sub_files) {
        if (i.name == file_name) {
            return i.id;
        }
    }
    return -1;
}

bool NTFS::checkFolder(qint64 mft_id) {
    qint64 entry_offset = mftOffset_ + mft_id * MFT_ENTRY_SIZE_;
    QByteArray file_flag = readInPartition(entry_offset + FILE_FLAG_OFFSET_, FILE_FLAG_LENGTH_);
    if (file_flag == QByteArrayLiteral("\x03\x00")) {
        return true;
    }
    return false;
}

qint64 NTFS::locatePath(QString path) {
    // If not found, return -1.
    QStringList file_list = path.split('/');
    qint64 id = ROOT_MFT_ID_;
    for (auto &i: file_list) {
        if (!checkFolder(id)) {
            return -1;
        }
        if (i.length() == 0) {
            continue;
        }
        QList<FileInfo> sub_files = listSubFiles(id);
        id = findFileInSubFiles_(sub_files, i);
        if (id == -1) {
            return -1;
        }
    }
    return id;
}

FileInfo NTFS::readFileInfo(qint64 mft_id) {
    // TODO: Other info.
    qint64 entry_offset = mftOffset_ + mft_id * MFT_ENTRY_SIZE_;
    QByteArray offset_data = readInPartition(
            entry_offset + FIRST_ATTRIBUTE_OFFSET_, FIRST_ATTRIBUTE_LENGTH_);
    qint64 offset = *(qint16 *) offset_data.constData() + entry_offset;
    NTFSAttribute attribute{};
    FileInfo info{};
    for (;;) {
        attribute = parseAttributeHead_(offset);
        if (attribute.type == END_TYPE_) {
            break;
        } else if (attribute.type == FILENAME_TYPE_) {
            QString temp = parseFileName_(offset + attribute.body_offset);
            if (temp.length() > info.name.length()) {
                info.name = temp;
            }
        } else if (attribute.type == FILE_CONTENT_TYPE_) {
            info.size = parseFileSize_(offset);
        }
        offset += attribute.length;
    }
    info.id = mft_id;
    return info;
}

FileInfo NTFS::readFileInfo(QString path) {
    qint64 id = locatePath(path);
    if (id == -1) {
        return {};
    }
    return readFileInfo(id);
}

QList<FileInfo> NTFS::listSubFiles(qint64 mft_id) {
    if (!checkFolder(mft_id)) {
        return {};
    }
    // TODO: will MFT has more data runs?
    // TODO: swift MFT reader to file reader.
    qint64 entry_offset = mftOffset_ + mft_id * MFT_ENTRY_SIZE_;
    QByteArray offset_data = readInPartition(
            entry_offset + FIRST_ATTRIBUTE_OFFSET_, FIRST_ATTRIBUTE_LENGTH_);
    qint64 offset = *(qint16 *) offset_data.constData() + entry_offset;
    NTFSAttribute attribute{};
    QList<FileInfo> ret{};
    for (;;) {
        attribute = parseAttributeHead_(offset);
        // TODO: are attributes always ordered?
        if (attribute.type == END_TYPE_ || attribute.type == INDEX_DISTRIBUTE_TYPE_) {
            break;
        } else if (attribute.type == INDEX_ROOT_TYPE_) {
            ret.append(parseIndex_(offset + attribute.body_offset, 0));
        }
        offset += attribute.length;
    }
    if (attribute.type == END_TYPE_) {
        return ret;
    }
    QList<NTFSDataRun> run_list = parseRunList_(attribute);
    for (auto &i: run_list) {
        ret.append(parseIndex_(i.start_cluster * clusterSize_, 1));
    }
    return ret;
}

QList<FileInfo> NTFS::listSubFiles(QString path) {
    qint64 id = locatePath(path);
    if (id == -1) {
        return {};
    }
    // Folder is checked in list sub files.
    return listSubFiles(id);
}

int NTFS::exportFile(qint64 mft_id, QString path) {
    // TODO: status code.
    // Now 1 for error and 0 for ok.
    if (!QDir(path).exists()) {
        return 1;
    }
    if (checkFolder(mft_id)) {
        FileInfo file_info = readFileInfo(mft_id);
        QString export_path = path;
        export_path.append("/");
        export_path.append(file_info.name);
        if (QDir(export_path).exists()) {
            return 1;
        }
        QDir().mkdir(export_path);
        QList<FileInfo> sub_files = listSubFiles(mft_id);
        for (auto &i: sub_files) {
            if (i.id == 0 or i.id == mft_id) {
                continue;
            }
            int status_code = exportFile(i.id, export_path);
            if (status_code) {
                return status_code;
            }
        }
        return 0;
    } else {
        FileInfo file_info = readFileInfo(mft_id);
        QString export_path = path;
        export_path.append("/");
        export_path.append(file_info.name);
        if (QFile(export_path).exists()) {
            return 1;
        }
        QFile output_file(export_path);
        if (!output_file.open(QIODevice::WriteOnly)) {
            return 1;
        }
        qint64 entry_offset = mftOffset_ + mft_id * MFT_ENTRY_SIZE_;
        QByteArray offset_data = readInPartition(
                entry_offset + FIRST_ATTRIBUTE_OFFSET_, FIRST_ATTRIBUTE_LENGTH_);
        qint64 offset = *(qint16 *) offset_data.constData() + entry_offset;
        NTFSAttribute attribute{};
        for (;;) {
            attribute = parseAttributeHead_(offset);
            if (attribute.type == END_TYPE_) {
                return 1;
            } else if (attribute.type == FILE_CONTENT_TYPE_) {
                break;
            }
            offset += attribute.length;
        }
        if (!attribute.resident) {
            output_file.write(readInPartition(offset + attribute.body_offset, attribute.body_length));
        } else {
            QList<NTFSDataRun> run_list = parseRunList_(attribute);
            // Total count for export actual size but not allocated size file.
            qint64 total_count = file_info.size;
            for (auto &i: run_list) {
                qint64 data_offset = i.start_cluster * clusterSize_;
                qint64 data_length = i.cluster_count * clusterSize_;
                while (data_length > 0 and total_count > 0) {
                    qint64 read_count = std::min(data_length, (qint64)256);
                    read_count = std::min(read_count, total_count);
                    output_file.write(readInPartition(data_offset, read_count));
                    data_length -= read_count;
                    total_count -= read_count;
                    data_offset += read_count;
                }
            }
        }
        output_file.close();
        return 0;
    }
}

int NTFS::exportFile(QString path, QString output_path) {
    qint64 id = locatePath(path);
    if (id == -1) {
        return {};
    }
    // Folder is checked in list sub files.
    return exportFile(id, output_path);
}
