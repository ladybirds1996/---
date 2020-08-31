#ifndef FRIENDLIST_H
#define FRIENDLIST_H

#include <QDialog>

namespace Ui {
class friendlist;
}

class friendlist : public QDialog
{
    Q_OBJECT

public:
    explicit friendlist(QWidget *parent = nullptr);
    ~friendlist();

private:
    Ui::friendlist *ui;
};

#endif // FRIENDLIST_H
