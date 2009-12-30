#ifndef _PLOT_H_
#define _PLOT_H_ 1

#include <qwt_plot.h>
#include <qwt_system_clock.h>
#include <qbasictimer.h>

class QwtPlotCurve;

class Plot: public QwtPlot
{
    Q_OBJECT

public:
    Plot(QWidget* = NULL);

public slots:
    void setTimerInterval(int);
    void setNumPoints(int);

protected:
    virtual void timerEvent(QTimerEvent *e);

private:
    void alignScales();

    QwtPlotCurve *d_curve;

    QBasicTimer d_timer;
    QwtSystemClock d_clock;
    double d_interval;
};

#endif
