#include "dcmlistthread.h"

DcmListThread::DcmListThread(QString& path, DcmContent& list) :
    path_(path),
    list_(list)
{
}

void DcmListThread::run()
{
    emit startToLoadFromFolder(path_, list_);
}
