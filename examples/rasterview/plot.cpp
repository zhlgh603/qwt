#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_layout.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_scale_widget.h>
#include "plot.h"

class RasterData: public QwtMatrixRasterData
{
public:
    RasterData()
    {
        const double matrix[] =
        {
            1, 2, 4, 1,
            6, 3, 5, 2,
            4, 2, 1, 5,
            5, 4, 2, 3 
        };

        QVector<double> values;
        for ( uint i = 0; i < sizeof(matrix) / sizeof(double); i++ )
            values += matrix[i];
    
        setMatrix(values, 4);

        setInterval( Qt::XAxis, 
            QwtInterval( -0.5, 3.5, QwtInterval::ExcludeMaximum ) );
        setInterval( Qt::YAxis, 
            QwtInterval( -0.5, 3.5, QwtInterval::ExcludeMaximum ) );
        setInterval( Qt::ZAxis, QwtInterval(1.0, 6.0) );
    }
};

class ColorMap: public QwtLinearColorMap
{
public:
    ColorMap():
        QwtLinearColorMap(Qt::darkBlue, Qt::darkRed)
    {
        addColorStop(0.2, Qt::blue);
        addColorStop(0.4, Qt::cyan);
        addColorStop(0.6, Qt::yellow);
        addColorStop(0.8, Qt::red);
    }
};

Plot::Plot(QWidget *parent):
    QwtPlot(parent)
{
    d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount(0); // use system specific thread count

    d_spectrogram->setColorMap( new ColorMap() );

    d_spectrogram->setData(new RasterData());
    d_spectrogram->attach(this);

	const QwtInterval zInterval = d_spectrogram->data()->interval( Qt::ZAxis );
    // A color bar on the right axis
    QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
    rightAxis->setColorBarEnabled(true);
    rightAxis->setColorBarWidth(40);
    rightAxis->setColorMap(zInterval, new ColorMap() );

    setAxisScale(QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
    enableAxis(QwtPlot::yRight);

    plotLayout()->setAlignCanvasToScales(true);

    setAxisScale(QwtPlot::xBottom, 0.0, 3.0);
    setAxisMaxMinor(QwtPlot::xBottom, 0);
    setAxisScale(QwtPlot::yLeft, 0.0, 3.0);
    setAxisMaxMinor(QwtPlot::yLeft, 0);
}

void Plot::setResampleMode(int mode)
{
    RasterData *data = (RasterData *)d_spectrogram->data();
    data->setResampleMode( (QwtMatrixRasterData::ResampleMode) mode);

    replot();
}
