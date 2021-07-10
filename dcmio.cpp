#include "dcmio.h"
#include <QDebug>
#include <QDir>

#include <cstdio>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/dcmdata/dctk.h>

DcmIO::DcmIO(QObject *parent) : QObject(parent)
{
}

DcmIO::~DcmIO()
{
}

bool DcmIO::LoadFromFolder(QString path, DcmContent &list)
{
    if (path.isEmpty())
    {
        qDebug() << "Folder path : " << path << " is invalid !";
        return false;
    }

    QFileInfoList files = SearchFilesFromAllFolders(path);
    int file_count = files.count();
    int progress_val = 0;

    for (int i = 0; i < file_count; ++i)
    {
        GetDcmDataset(files.at(i).absoluteFilePath(), list);
        emit progress(100 * (++progress_val) / file_count);
    }
    qDebug() << "Load Folder success !";
    return true;
}

void DcmIO::GetDcmDataset(QString path, DcmContent& list)
{
    DcmFileFormat* format = new DcmFileFormat();
    OFCondition result = format->loadFile(OFFilename(path.toLocal8Bit()));
    if (result.bad())
    {
        qDebug() << "Failed to load " << path << " : ";
        qDebug() << result.text();
        delete format;
        return;
    }

    OFString str;
    if (format->getDataset()->isEmpty())
    {
        qDebug() << "Failed to get dataset " << path << " : ";
        delete format;
        return;
    }

    DcmInstance instance;
    instance.file_path_ = path;
    result = format->getDataset()->findAndGetOFString(DCM_SOPInstanceUID, str);
    instance.sop_instance_uid_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_InstanceNumber, str);
    instance.instance_number_ = (result.good()) ? std::stoi(str.c_str()) : -1;
    result = format->getDataset()->findAndGetOFString(DCM_NumberOfFrames, str);
    instance.number_of_frame_ = (result.good()) ? std::stoi(str.c_str()) : 0;

    DcmSeries series;
    result = format->getDataset()->findAndGetOFString(DCM_SeriesInstanceUID, str);
    series.series_instance_uid_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_SeriesDescription, str);
    series.series_description_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_SeriesNumber, str);
    series.series_number_ = (result.good()) ? std::stoi(str.c_str()) : -1;
    series.instance_list_.push_back(instance);

    DcmStudy study;
    result = format->getDataset()->findAndGetOFString(DCM_StudyInstanceUID, str);
    study.study_instance_uid_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_StudyDescription, str);
    study.study_description_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_StudyDate, str);
    study.study_date_ = (result.good()) ? str.c_str() : "";
    study.series_list_.push_back(series);

    DcmPatient patient;
    result = format->getDataset()->findAndGetOFString(DCM_PatientName, str);
    patient.patient_name_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_PatientID, str);
    patient.patient_id_ = (result.good()) ? str.c_str() : "";
    result = format->getDataset()->findAndGetOFString(DCM_PatientBirthDate, str);
    patient.patient_bd_ = (result.good()) ? str.c_str() : "";
    patient.study_list_.push_back(study);

    auto it_patient = std::find_if(list.begin(), list.end(), [&](DcmPatient& p) {
        return p.patient_name_ == patient.patient_name_;
    });
    if (it_patient == list.end())
    {
        list.push_back(patient);
        delete format;
        return;
    }

    auto it_study = std::find_if(it_patient->study_list_.begin(), it_patient->study_list_.end(), [&](DcmStudy& s){
        return s.study_instance_uid_ == study.study_instance_uid_;
    });
    if (it_study == it_patient->study_list_.end())
    {
        it_patient->study_list_.push_back(study);
        delete format;
        return;
    }

    auto it_series = std::find_if(it_study->series_list_.begin(), it_study->series_list_.end(), [&](DcmSeries& s){
       return s.series_instance_uid_ == series.series_instance_uid_;
    });
    if (it_series == it_study->series_list_.end())
    {
        it_study->series_list_.push_back(series);
        std::sort(it_study->series_list_.begin(), it_study->series_list_.end(), [](DcmSeries& s1, DcmSeries& s2){
            return s1.series_number_ < s2.series_number_;
        });
        delete format;
        return;
    }

    auto it_instance = std::find_if(it_series->instance_list_.begin(), it_series->instance_list_.end(), [&](DcmInstance& i){
        return i.sop_instance_uid_ == instance.sop_instance_uid_;
    });
    if (it_instance == it_series->instance_list_.end())
    {
        it_series->instance_list_.push_back(instance);
        std::sort(it_series->instance_list_.begin(), it_series->instance_list_.end(), [](DcmInstance& i1, DcmInstance& i2){
            return i1.instance_number_ < i2.instance_number_;
        });
    }

    delete format;
    return;
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
