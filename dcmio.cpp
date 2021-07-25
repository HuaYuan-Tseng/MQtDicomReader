#include "dcmio.h"
#include <QDebug>
#include <QDir>

#include <cstdio>
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmjpeg/djdecode.h>
#include <dcmtk/dcmdata/dcrledrg.h>
#include <dcmtk/dcmimgle/dcmimage.h>

DcmIO::DcmIO(QObject *parent) : QObject(parent)
{
}

DcmIO::~DcmIO()
{
}

bool DcmIO::LoadFromFolder(QString* path, DcmContent* list)
{
    if (path->isEmpty())
    {
        qDebug() << "Folder path : " << path << " is invalid !";
        return false;
    }

    QFileInfoList files = SearchFilesFromAllFolders(*path);
    int file_count = files.count();
    int progress_val = 0;

    for (int i = 0; i < file_count; ++i)
    {
        GetDcmMetaData(files.at(i).absoluteFilePath(), *list);
        emit progress(100 * (++progress_val) / file_count);
    }
    emit send(path, list);
    emit finish();

    qDebug() << "Load Folder success !";
    return true;
}

void DcmIO::GetDcmMetaData(QString path, DcmContent& list)
{
    DcmFileFormat* format = new DcmFileFormat();
    OFCondition result = format->loadFile(OFFilename(path.toLocal8Bit()));
    if (result.bad())
    {
        qDebug() << "Failed to load " << path << " : ";
        //qDebug() << result.text();
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
    instance.number_of_frame_ = (result.good()) ? std::stoi(str.c_str()) : 1;

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

bool DcmIO::LoadInstanceDataSet(std::vector<DcmInstance>* list, DcmDataSet* data_set)
{
    if (list->empty()) return false;

    GetInstanceMetaData(list->at(0), *data_set);
    data_set->set_total_instances(static_cast<int>(list->size()));
    int file_count = static_cast<int>(list->size());
    int progress_val = 0;

    std::vector<double> location_list;
    location_list.reserve(list->size());
    emit progress(100 * (++progress_val) / (file_count + 1));
    for (int i = 0; i < file_count; ++i)
    {
        double location = 0.0;
        GetInstanceDataSet(list->at(i), *data_set, location);
        location_list.push_back(location);
        emit progress(100 * (++progress_val) / (file_count + 1));
    }
    if (!location_list.empty() &&
            std::abs(location_list[0] - location_list[1]) == std::abs(location_list[1] - location_list[2]))
    {
        data_set->set_spacing_z(std::abs(location_list[0] - location_list[1]));
    }
    emit send(data_set);
    emit finish();

    return true;
}

void DcmIO::GetInstanceDataSet(DcmInstance& instance, DcmDataSet& data_set, double& location)
{
    DcmFileFormat* format = new DcmFileFormat();
    OFCondition result = format->loadFile(OFFilename(instance.file_path_.toLocal8Bit()));
    if (result.bad()) return;

    std::string loss_less_trans_uid = "1.2.840.10008.1.2.4.70";
    std::string loss_trans_uid = "1.2.840.10008.1.2.4.51";
    std::string loss_less_p14 = "1.2.840.10008.1.2.4.57";
    std::string lossy_p1 = "1.2.840.10008.1.2.4.50";
    std::string lossy_rle = "1.2.840.10008.1.2.5";

    //E_TransferSyntax transfer = format->getDataset()->getOriginalXfer();

    OFString str;
    result = format->getDataset()->findAndGetOFString(DCM_SliceLocation, str);
    if (result.good())  location = std::stod(str.c_str());
    else                location = 0.0;

    bool is_compressed = false;
    const char* syntax = nullptr;
    format->getMetaInfo()->findAndGetString(DCM_TransferSyntaxUID, syntax);

    if (syntax == nullptr)
    {
        is_compressed = false;
    }
    else
    {
        if (syntax == loss_less_trans_uid || syntax == loss_trans_uid ||
                syntax == loss_less_p14 || syntax == lossy_p1)
        {
            DJDecoderRegistration::registerCodecs();
            format->getDataset()->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);
            DJDecoderRegistration::cleanup();
            is_compressed = true;
        }
        else if (syntax == lossy_rle)
        {
            DcmRLEDecoderRegistration::registerCodecs();
            format->getDataset()->chooseRepresentation(EXS_LittleEndianExplicit, nullptr);
            DcmRLEDecoderRegistration::cleanup();
            is_compressed = true;
        }
    }

    DicomImage* dcm_image = nullptr;
    if (is_compressed)
    {
        dcm_image = new DicomImage((DcmObject*)(format->getDataset()),
                                               format->getDataset()->getOriginalXfer(),
                                               CIF_TakeOverExternalDataset);
        if (dcm_image->getStatus() == EIS_Normal)
        {
            // getOutputDataSize() is just for single frame.
            const int frame_size = data_set.frames_per_instance();
            const ulong img_size = dcm_image->getOutputDataSize(data_set.bits_stored());
            uchar* dst_ptr = new uchar[frame_size * img_size];
            
            for (int i = 0; i < frame_size; ++i)
            {
                uchar* img_ptr = (uchar*)(dcm_image->getOutputData(data_set.bits_stored(), i));
                std::memcpy(dst_ptr + i * img_size, img_ptr, img_size);
            }
            
            data_set.set_instance_raw_data(dst_ptr);
        }
    }
    else
    {
        dcm_image = new DicomImage(format,
                                   format->getDataset()->getOriginalXfer(),
                                   CIF_TakeOverExternalDataset);
        if (dcm_image->getStatus() == EIS_Normal)
        {
            // getOutputDataSize() is just for single frame.
            const int frame_size = data_set.frames_per_instance();
            const ulong img_size = dcm_image->getOutputDataSize(data_set.bits_stored());
            uchar* dst_ptr = new uchar[frame_size * img_size];
            
            for (int i = 0; i < frame_size; ++i)
            {
                uchar* img_ptr = (uchar*)(dcm_image->getOutputData(data_set.bits_stored(), i));
                std::memcpy(dst_ptr + i * img_size, img_ptr, img_size);
            }
            
            data_set.set_instance_raw_data(dst_ptr);
        }
    }
    delete dcm_image;
}

void DcmIO::GetInstanceMetaData(DcmInstance& instance, DcmDataSet& data_set)
{
    DcmFileFormat* format = new DcmFileFormat();
    OFCondition result = format->loadFile(OFFilename(instance.file_path_.toLocal8Bit()));
    if (result.bad()) return;

    OFString str;   double value;
    if (format->getDataset()->isEmpty()) return;

    result = format->getDataset()->findAndGetOFString(DCM_PatientName, str);
    if (result.good()) data_set.set_patient_name(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_PatientID, str);
    if (result.good()) data_set.set_patient_id(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_StudyInstanceUID, str);
    if (result.good()) data_set.set_study_instance_uid(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_StudyDescription, str);
    if (result.good()) data_set.set_study_description(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_SeriesInstanceUID, str);
    if (result.good()) data_set.set_series_instance_uid(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_SeriesDescription, str);
    if (result.good()) data_set.set_series_description(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_SeriesNumber, str);
    if (result.good()) data_set.set_series_number(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_SOPInstanceUID, str);
    if (result.good()) data_set.set_sop_instance_uid(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_SOPClassUID, str);
    if (result.good()) data_set.set_sop_class_uid(str.c_str());

    result = format->getDataset()->findAndGetOFString(DCM_Rows, str);
    if (result.good()) data_set.set_rows(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_Columns, str);
    if (result.good()) data_set.set_cols(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_NumberOfFrames, str);
    if (result.good()) data_set.set_frames_per_instance(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetFloat64(DCM_PixelSpacing, value, 0);
    if (result.good()) data_set.set_spacing_x(value);

    result = format->getDataset()->findAndGetFloat64(DCM_PixelSpacing, value, 1);
    if (result.good()) data_set.set_spacing_y(value);

    result = format->getDataset()->findAndGetOFString(DCM_SliceThickness, str);
    if (result.good()) data_set.set_slice_thickness(std::stod(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_SpacingBetweenSlices, str);
    if (result.good()) data_set.set_spacing_between_slice(std::stod(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_PixelRepresentation, str);
    if (result.good()) data_set.set_pixel_representation(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_RescaleIntercept, str);
    if (result.good()) data_set.set_rescale_intercept(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_RescaleSlope, str);
    if (result.good()) data_set.set_rescale_slope(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_BitsAllocated, str);
    if (result.good()) data_set.set_bits_allocated(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_BitsStored, str);
    if (result.good()) data_set.set_bits_stored(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetOFString(DCM_HighBit, str);
    if (result.good()) data_set.set_high_bit(std::stoi(str.c_str()));

    result = format->getDataset()->findAndGetFloat64(DCM_WindowWidth, value, 0);
    if (result.good()) data_set.set_window_width(value);

    result = format->getDataset()->findAndGetFloat64(DCM_WindowWidth, value, 1);
    if (result.good()) data_set.set_window_width(value);

    result = format->getDataset()->findAndGetFloat64(DCM_WindowCenter, value, 0);
    if (result.good()) data_set.set_window_center(value);

    result = format->getDataset()->findAndGetFloat64(DCM_WindowCenter, value, 1);
    if (result.good()) data_set.set_window_center(value);

    if (data_set.window_width().empty() || data_set.window_center().empty())
    {
        int width = std::pow(2, data_set.bits_allocated());
        int center = width / 2;
        data_set.set_window_width(width);
        data_set.set_window_center(center);
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
