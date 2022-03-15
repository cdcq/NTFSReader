//
// Created by cdcq on 2021/11/26.
//

#include "controller.h"

#include <QDebug>

#include "layers/disk/disk.h"
#include "layers/filesystem/filesystem.h"
#include "layers/image/image.h"

void Controller::run() {
    QTextStream in(stdin);
    QTextStream out(stdout);
    out << "Which file do you want to read:" << Qt::endl;
    QString file_name;
    // file_name = "/run/media/cdcq/Samsung_T5/Win10Basic/Win10Basic_e01.E01";
    // file_name = "/run/media/cdcq/Samsung_T5/Win10ExportTest/export_test.E01";
    file_name = "/run/media/cdcq/Samsung_T5/Win7Basic/Win7Basic.E01";
    // file_name = in.readLine();
    QFile temp_file(file_name);
    if (!temp_file.open(QIODevice::ReadOnly)) {
        out << "The file could not to open! :(" << Qt::endl;
        return;
    }
    temp_file.close();
    fileName_ = file_name;

    ImageReader *image = ImageFactor::createImageReader(file_name);
    if (image == nullptr) {
        out << "The file type is not supported! >n<" << Qt::endl;
        return;
    }
    class DiskReader *disk = DiskFactor::createDiskReader(image);
    if (disk == nullptr) {
        out << "The disk type is not supported! >n<" << Qt::endl;
    }
    QList<PartitionInfo> partitions = disk->partitions();
    for (int i = 0; i < partitions.size(); i++) {
        out << i << ". " << partitions[i].name << Qt::endl;
    }
    out << "Choose your partition (default 2): ";
    out.flush();
    int partition_id;
    QString inp;
    inp = in.readLine();
    if (inp.length() == 0) {
        partition_id = 2;
    } else {
        partition_id = inp.toInt();
    }
    if (partition_id >= partitions.size() || partition_id < 0) {
        out << "What are you doing? :(" << Qt::endl;
        return;
    }
    FilesystemReader *filesystem = FilesystemFactor::createFilesystemReader(partition_id, disk);
    if (filesystem == nullptr) {
        out << "The filesystem type is not supported! >n<" << Qt::endl;
        return;
    }

    QString path("/");
    for (;;) {
        out << path << " >> ";
        out.flush();
        inp = in.readLine();
        if (inp == QString("exit")) {
            break;
        } else if (inp == QString("ls")) {
            QList<FileInfo> sub_files = filesystem->listSubFiles(path);
            for (auto &i: sub_files) {
                /* if (i.name.length() == 0 || i.name[0] == '$') {
                    continue;
                } */
                out << i.name << " " << i.size << " " << i.id << Qt::endl;
            }
        } else if (inp.startsWith("cd ")) {
            inp = inp.mid(3, inp.length() - 3);
            if (inp == QString("..")) {
                int index;
                for (index = path.size() - 1; index >= 0 && path[index] != '/'; index--);
                if (index > 0) {
                    path = path.mid(0, index);
                }
                continue;
            }
            QList<FileInfo> sub_files = filesystem->listSubFiles(path);
            qint64 id = 0;
            for (auto &i: sub_files) {
                if (i.name == inp) {
                    id = i.id;
                    break;
                }
            }
            if (!id || !filesystem->checkFolder(id)) {
                out << "\"" << inp << "\" is not a sub folder." << Qt::endl;
            } else {
                path.append("/");
                path.append(inp);
            }
        }
    }
}
