//
// Created by cdcq on 2021/11/26.
//

#include "e01.h"

E01Section::E01Section() {
    type_ = E01SectionType::Unknown;
    next_ = -1;
    size_ = -1;
    checksum_ = -1;
}

void E01Section::parseDescriptor(const QByteArray &descriptor) {
    if (descriptor.length() != SECTION_HEAD_SIZE_) {
        // TODO: exception.
        return;
    }
    QByteArray type = descriptor.mid(0, 8);
    if (type == QByteArrayLiteral("header\x00\x00")) {
        type_ = E01SectionType::Header;
    } else if (type == QByteArrayLiteral("table\x00\x00\x00")) {
        type_ = E01SectionType::Table;
    } else if (type == QByteArrayLiteral("done\x00\x00\x00\x00")) {
        type_ = E01SectionType::Done;
    } else if (type == QByteArrayLiteral("disk\x00\x00\x00\x00")) {
        type_ = E01SectionType::Disk;
    } else if (type == QByteArrayLiteral("sectors\x00")) {
        type_ = E01SectionType::Sectors;
    } else if (type == QByteArrayLiteral("table2\x00\x00")) {
        type_ = E01SectionType::Table2;
    } else if (type == QByteArrayLiteral("hash\x00\x00\x00\x00")) {
        type_ = E01SectionType::Hash;
    } else if (type == QByteArrayLiteral("digest\x00\x00")) {
        type_ = E01SectionType::Digest;
    } else {
        // TODO: warning.
        type_ = E01SectionType::Unknown;
    }
    // TODO: more types support.
    next_ = *(qint64*)descriptor.mid(NEXT_OFFSET_, NEXT_LENGTH_).constData();
    size_ = *(qint64*)descriptor.mid(SIZE_OFFSET_, SIZE_LENGTH_).constData();
    checksum_ = *(qint32*)descriptor.mid(CHECK_SUM_OFFSET_, CHECK_SUM_LENGTH_).constData();
}

E01SectionType E01Section::type() const {
    return type_;
}

qint64 E01Section::next() const {
    return next_;
}

qint64 E01Section::size() const {
    return size_;
}

void E01Image::load_() {
    // TODO: how to avoid wrong reload after change file name?
    // TODO: multi files support.
    qint64 offset = EWF_HEAD_SIZE_;
    qint64 size_sum = 0;
    for (;;) {
        E01Section section;
        section.parseDescriptor(readInFile_(offset, SECTION_HEAD_SIZE_));
        if (section.type() == E01SectionType::Done || section.size() <= 0) {
            // TODO: warning.
            break;
        } else if (section.type() == E01SectionType::Table) {
            QByteArray count_data = readInFile_(offset + SECTION_HEAD_SIZE_, ENTRY_COUNT_SIZE_);
            qint64 count = *(qint32*)count_data.constData();
            QByteArray base_data = readInFile_(offset + TABLE_BASE_OFFSET_, TABLE_BASE_LENGTH_);
            qint64 base = *(qint64*)base_data.constData();
            E01Table table{offset, count, count * chunkSize_, size_sum, base};
            size_sum += table.size;
            tables_.append(table);
        }
        offset = section.next();
    }
}

E01Image::E01Image(const QString &file_name) {
    if (!checkE01File(file_name)) {
        return;
    }
    sectorCount_ = 64;
    sectorSize_ = 512;
    chunkSize_ = sectorCount_ * sectorSize_;
    setFileName(file_name);

    load_();
}

bool E01Image::checkE01File(const QString &file_name) {
    QFile file(file_name);

    if (!file.open(QIODevice::ReadOnly)) {
        // TODO: exception.
        return false;
    }
    // Attention! Following code is wrong, in which '\x00' will be removed.
    // if (file.read(8).compare("EVF\x09\x0d\x0a\xff\x00")) {
    if (file.read(8).compare(QByteArray::fromHex("455646090d0aff00"))) {
        // TODO: exception.
        return false;
    }

    file.close();
    return true;
}

int E01Image::locateTable_(qint64 offset) {
    int left = 0, right = tables_.size() - 1, mid = 0;
    while (left + 1 < right) {
        mid = (left + right) >> 1;
        if (tables_[mid].offset <= offset) {
            left = mid;
        } else {
            right = mid;
        }
    }
    if (tables_[right].offset <= offset) {
        return right;
    } else {
        return left;
    }
}

qint64 E01Image::locateChunk_(E01Table table, qint64 location) {
    qint64 entry_offset = table.start + SECTION_HEAD_SIZE_ + TABLE_HEAD_SIZE_ + TABLE_ENTRY_SIZE_ * location;
    return *(qint32*)readInFile_(entry_offset, TABLE_ENTRY_SIZE_).constData() + table.base;
}

QByteArray E01Image::readInFile_(qint64 offset, qint64 count) {
    // TODO: optimize, such global file.
    QFile file(fileName());
    if (!file.open(QIODevice::ReadOnly)) {
        // TODO: warning.
        return {""};
    }
    file.seek(offset);
    QByteArray ret = file.read(count);
    file.close();
    return ret;
}

QByteArray E01Image::readInTable_(E01Table table, qint64 offset, qint64 count) {
    // TODO: optimize, such as turn the table to a const reference.
    qint64 location = offset / chunkSize_;
    qint64 chunk_offset = locateChunk_(table, location);
    qint64 read_offset = offset % chunkSize_;
    qint64 read_count = qMin(chunkSize_ - read_offset, count);
    QByteArray ret = readInFile_(chunk_offset + read_offset, read_count);
    count -= read_count;
    while (count > 0) {
        location++;
        if (location == table.count) {
            // TODO: warning.
            return {""};
        }
        read_count = qMin(count, chunkSize_);
        ret += readInFile_(locateChunk_(table, location), read_count);
        count -= read_count;
    }
    return ret;
}

QByteArray E01Image::read(qint64 offset, qint64 count) {
    if (offset >= tables_.last().offset + tables_.last().size) {
        // TODO: warning.
        return {""};
    }
    int location = locateTable_(offset);
    E01Table table = tables_[location];
    qint64 read_offset = offset - table.offset;
    qint64 read_count = qMin(table.size - read_offset, count);
    QByteArray ret = readInTable_(table, read_offset, read_count);
    count -= read_count;
    while (count > 0) {
        location++;
        if (location == tables_.length()) {
            // TODO: warning.
            return {""};
        }
        read_count = qMin(count, tables_[location].size);
        ret += readInTable_(tables_[location], 0, read_count);
        count -= read_count;
    }
    return ret;
}
