#ifndef DCMLISTTHREAD_H
#define DCMLISTTHREAD_H

#include <QThread>

#include "dcmlayer.h"

class DcmListThread : public QThread
{
    Q_OBJECT
public:
    explicit        DcmListThread(const QString& path, const DcmContent& list);

protected:
    virtual void    run() override;

signals:
    void            startToLoadFromFolder(QString* path, DcmContent* list);

private:
    QString         path_;
    DcmContent      list_;
};

#endif // DCMLISTTHREAD_H
