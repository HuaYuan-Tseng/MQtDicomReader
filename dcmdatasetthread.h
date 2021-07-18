#ifndef DCMDATASETTHREAD_H
#define DCMDATASETTHREAD_H

#include <QThread>

#include "dcmlayer.h"
#include "dcmdataset.h"

class DcmDatasetThread : public QThread
{
    Q_OBJECT
public:
    explicit        DcmDatasetThread(std::vector<DcmInstance>& instance_list, DcmDataSet& data_set);

protected:
    virtual void    run() override;

signals:
    void            startToLoadInstanceDataSet(std::vector<DcmInstance>& instance_list, DcmDataSet& data_set);

private:
    std::vector<DcmInstance>    instance_list_;
    DcmDataSet                  data_set_;

};

#endif // DCMDATASETTHREAD_H
