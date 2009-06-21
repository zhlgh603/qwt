#ifndef _WHEELBOX_H_
#define _WHEELBOX_H_

#include <qwidget.h>

class QwtWheel;
class QLabel;
class QLCDNumber;

class WheelBox: public QWidget
{
    Q_OBJECT

public:
    WheelBox(const QString &title, 
        double min, double max, double stepSize, 
        QWidget *parent = NULL);

    void setUnit(const QString &);
    QString unit() const;

    void setValue(double value);
    double value() const;

signals:
    double valueChanged(double);

private:
    QLCDNumber *d_number;
    QwtWheel *d_wheel;
    QLabel *d_label;

    QString d_unit;
};

#endif
