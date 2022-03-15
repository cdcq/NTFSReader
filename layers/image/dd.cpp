//
// Created by cdcq on 2021/11/27.
//

#include "dd.h"

DDImage::DDImage(const QString &file_name) {
    setFileName(file_name);
}

QByteArray DDImage::read(qint64 offset, qint64 count) {
    QFile file(fileName());
    if (!file.open(QIODevice::ReadOnly)) {
        // TODO: throw.
        return QByteArray{};
    }
    file.seek(offset);
    QByteArray ret = file.read(count);
    file.close();
    return ret;
}
