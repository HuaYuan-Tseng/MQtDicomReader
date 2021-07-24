#include "dcmdatasetthread.h"

DcmDatasetThread::DcmDatasetThread(std::vector<DcmInstance>& instance_list, DcmDataSet& data_set) :
    instance_list_(instance_list),
    data_set_(data_set)
{

}

void DcmDatasetThread::run()
{
    emit startToLoadInstanceDataSet(&instance_list_, &data_set_);
}
