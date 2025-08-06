#ifndef LISTITEMBASE_H
#define LISTITEMBASE_H
#include <QWidget>
#include "global.h"

//条目的基类，到时候子类条目都继承他就可以。
class ListItemBase:public QWidget
{
    Q_OBJECT
public:
    explicit ListItemBase(QWidget *parent = nullptr);
    void SetItemType(ListItemType itemType);

    ListItemType GetItemType();
private:
    ListItemType _itemType;

public slots:

signals:

};

#endif // LISTITEMBASE_H
