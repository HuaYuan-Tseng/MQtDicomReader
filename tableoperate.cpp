#include "tableoperate.h"

void TableOperate::SetTableHeader(QStandardItemModel* model, std::vector<QString> header)
{
    int size = static_cast<int>(header.size());
    for (int i = 0; i < size; ++i)
    {
        model->setHeaderData(i, Qt::Horizontal, header.at(i));
    }
}

void TableOperate::RefreshTableContents(QStandardItemModel* model, std::vector<std::vector<QString>> contents)
{
    int row = static_cast<int>(contents.size());
    for (int i = 0; i < row; ++i)
    {
        int col = static_cast<int>(contents[i].size());
        for (int j = 0; j < col; ++j)
        {
            model->setItem(i, j, new QStandardItem(contents[i][j]));
            //model->setData(model->index(i, j, QModelIndex()), Qt::AlignLeft, Qt::TextAlignmentRole);
            //model->setData(model->index(i, j, QModelIndex()), contents[i][j]);
        }
    }
}
