#ifndef DCMIO_H
#define DCMIO_H

#include <QObject>
#include <QFileInfoList>

#include "dcmlayer.h"

class DcmIO : public QObject
{
    Q_OBJECT
public:
    explicit    DcmIO(QObject* parent = nullptr);
                ~DcmIO();

    bool                LoadFromFolder(QString path, DcmList& list);

private:
    QFileInfoList       SearchFilesFromAllFolders(QString path);

signals:
    void        progress(int);      // loading progress
};

#endif // DCMIO_H
