#ifndef DCMIO_H
#define DCMIO_H

#include <QObject>
#include <QFileInfoList>

#include "dcmlayer.h"
#include "globalstate.h"

class DcmIO : public QObject
{
    Q_OBJECT
public:
    DcmIO(QObject* parent = nullptr);
    ~DcmIO();

private:
    QFileInfoList       SearchFilesFromAllFolders(QString path);
    void                GetDcmDataset(QString path, DcmContent& list);

public slots:
    bool                LoadFromFolder(QString& path, DcmContent& list);

signals:
    void                send(QString& path, DcmContent& list);  // send data from dcmio's thread
    void                progress(int);                          // loading progress
    void                finish();
};

#endif // DCMIO_H
