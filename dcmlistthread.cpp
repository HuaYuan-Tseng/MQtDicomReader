#include "dcmlistthread.h"

DcmListThread::DcmListThread(const QString& path, const DcmContent& list) :
    path_(path),
    list_(list)
{
}

void DcmListThread::run()
{
    emit startToLoadFromFolder(&path_, &list_);
}
