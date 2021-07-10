#ifndef DCMIO_H
#define DCMIO_H

#include <QObject>
#include <QFileInfoList>

#include "dcmlayer.h"

class DcmIO : public QObject
{
    Q_OBJECT
public:
    DcmIO(QObject* parent = nullptr);
    ~DcmIO();

    bool                LoadFromFolder(QString path, DcmContent& list);

private:
    QFileInfoList       SearchFilesFromAllFolders(QString path);
    void                GetDcmDataset(QString path, DcmContent& list);

signals:
    void                progress(int);      // loading progress
};

#endif // DCMIO_H
