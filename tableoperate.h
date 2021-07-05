#ifndef TABLEOPERATE_H
#define TABLEOPERATE_H

#include <QStandardItemModel>

class TableOperate {
public:
    static void SetTableHeader(QStandardItemModel* model, std::vector<QString> header);
};

#endif // TABLEOPERATE_H
