#include "dcmio.h"
#include <QDebug>
#include <QDir>

DcmIO::DcmIO(QObject *parent) : QObject(parent)
{
}

DcmIO::~DcmIO()
{
}

bool DcmIO::LoadFromFolder(QString path, DcmList &list)
{
    if (path.isEmpty())
    {
        qDebug() << "Folder path is invalid !";
        return false;
    }

    QFileInfoList file = SearchFilesFromAllFolders(path);
    int file_count = file.count();
    int progress_val = 0;

    for (int i = 0; i < file_count; ++i)
    {


        emit progress(100 * (++progress_val) / file_count);
    }

    return true;
}

QFileInfoList DcmIO::SearchFilesFromAllFolders(QString path)
{
    QDir dir(path);
    QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for(int i = 0; i != folder_list.size(); i++)
    {
         QString name = folder_list.at(i).absoluteFilePath();
         QFileInfoList child_file_list = SearchFilesFromAllFolders(name);
         file_list.append(child_file_list);
    }
    return file_list;
}
