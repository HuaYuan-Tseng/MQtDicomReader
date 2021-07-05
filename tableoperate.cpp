#include "tableoperate.h"

void TableOperate::SetTableHeader(QStandardItemModel* model, std::vector<QString> header)
{
    int size = static_cast<int>(header.size());
    for (int i = 0; i < size; ++i)
    {
        model->setHeaderData(i, Qt::Horizontal, header.at(i));
    }
}
