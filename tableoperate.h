#ifndef TABLEOPERATE_H
#define TABLEOPERATE_H

#include <QStandardItemModel>

class TableOperate {
public:
    static void SetTableHeader(QStandardItemModel* model, std::vector<QString> header);
    static void RefreshTableContents(QStandardItemModel* model, std::vector<std::vector<QString>> contents);
};

#endif // TABLEOPERATE_H
