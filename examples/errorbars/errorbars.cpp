#include <qapplication.h>
#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_intervalcurve.h>
#include <qwt_legend.h>
#include <qwt_interval_symbol.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_text.h>
#include "friedberg2007.h"

class FriedbergPlot: public QwtPlot
{
public:
    FriedbergPlot();

private:
    void insertCurve(const QString& title,
        const QwtArray<QwtDoublePoint>&, const QColor &);

    void insertErrorBars(const QwtArray<QwtIntervalSample>&,
        const QColor &color, bool showTube);
};

FriedbergPlot::FriedbergPlot()
{
    setTitle("Temperature of Friedberg/Germany");
    setCanvasBackground(QColor(Qt::darkGray));

    setAxisTitle(QwtPlot::xBottom, "2007");
    setAxisScale(QwtPlot::xBottom, 0.0, 365.0);
    setAxisTitle(QwtPlot::yLeft, 
        QString("Temperature [%1C]").arg(QChar(0x00B0)) );
    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajPen(QPen(Qt::white, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(this);

    insertLegend(new QwtLegend(), QwtPlot::RightLegend);

    const int numDays = 365;
    QwtArray<QwtDoublePoint> averageData(numDays);
    QwtArray<QwtIntervalSample> rangeData(numDays);

    for ( int i = 0; i < numDays; i++ )
    {
        const Temperature &t = friedberg2007[i];
        averageData[i] = QwtDoublePoint(double(i), t.averageValue);
        rangeData[i] = QwtIntervalSample( double(i),
            QwtDoubleInterval(t.minValue, t.maxValue) );
    }

    insertCurve("Average", averageData, Qt::black);
    insertErrorBars(rangeData, Qt::blue, true);

    QwtPlotZoomer* zoomer = new QwtPlotZoomer(canvas());
    zoomer->setRubberBandPen(QColor(Qt::black));
    zoomer->setTrackerPen(QColor(Qt::black));
}

void FriedbergPlot::insertCurve(const QString& title, 
    const QwtArray<QwtDoublePoint>& samples, const QColor &color)
{
    QwtPlotCurve *curve = new QwtPlotCurve(title);
#if QT_VERSION >= 0x040000
    curve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    curve->setStyle(QwtPlotCurve::NoCurve);

    QwtSymbol symbol;
    symbol.setStyle(QwtSymbol::XCross);
    symbol.setSize(4);
    symbol.setPen(QPen(color));
    curve->setSymbol(symbol);

    curve->setSamples(samples);
    curve->attach(this);
}

void FriedbergPlot::insertErrorBars(
    const QwtArray<QwtIntervalSample>& samples, 
    const QColor &color, bool showTube)
{
    QwtPlotIntervalCurve *errorCurve = new QwtPlotIntervalCurve();
#if QT_VERSION >= 0x040000
    errorCurve->setRenderHint(QwtPlotItem::RenderAntialiased);
#endif
    errorCurve->setPen(QPen(Qt::white));

    if ( showTube )
    {
        QColor bg(Qt::white);
#if QT_VERSION >= 0x040000
        bg.setAlpha(150);
#endif
        errorCurve->setBrush(QBrush(bg));
        errorCurve->setCurveStyle(QwtPlotIntervalCurve::Tube);
    }
    else
    {
        errorCurve->setCurveStyle(QwtPlotIntervalCurve::NoCurve);
    }

    QwtIntervalSymbol errorBar(QwtIntervalSymbol::Bar);
    errorBar.setWidth(7);
    errorBar.setPen(QPen(color));
    errorCurve->setSymbol(errorBar);

    errorCurve->setSamples(samples);
    errorCurve->attach(this);
}

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    FriedbergPlot plot;
#if QT_VERSION < 0x040000
    a.setMainWidget(&plot);
#endif
    plot.resize(600,400);
    plot.show();
    return a.exec(); 
}
