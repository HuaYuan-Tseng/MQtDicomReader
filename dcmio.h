#ifndef DCMIO_H
#define DCMIO_H

#include <QObject>
#include <QFileInfoList>

#include "dcmlayer.h"
#include "dcmdataset.h"
#include "globalstate.h"

class DcmIO : public QObject
{
    Q_OBJECT
public:
    DcmIO(QObject* parent = nullptr);
    ~DcmIO();

private:
    QFileInfoList       SearchFilesFromAllFolders(QString path);
    void                GetDcmMetaData(QString path, DcmContent& list);

    void                GetInstanceMetaData(DcmInstance& instance, DcmDataSet& data_set);
    void                GetInstanceDataSet(DcmInstance& instance, DcmDataSet& data_set, double& location);

public slots:
    bool                LoadFromFolder(QString* path, DcmContent* list);
    bool                LoadInstanceDataSet(std::vector<DcmInstance>* list, DcmDataSet* data_set);

signals:
    void                send(DcmDataSet* data_set);
    void                send(QString* path, DcmContent* list);  // send data from dcmio's thread
    void                progress(int);                          // loading progress
    void                finish();
};

#endif // DCMIO_H
