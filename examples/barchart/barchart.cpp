#include "barchart.h"
#include <qwt_plot_renderer.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_barchart.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_draw.h>
#include <qfiledialog.h>
#include <qimagewriter.h>
#include <qprintdialog.h>
#include <qevent.h>
#include <qfileinfo.h>

BarChart::BarChart( QWidget *parent ):
    QwtPlot( parent )
{
    setTitle( "Bar Chart" );

    canvas()->setPalette( Qt::gray );
    canvas()->installEventFilter( this );

    setAxisTitle( QwtPlot::yLeft, "Whatever" );
    setAxisTitle( QwtPlot::xBottom, "Whatever" );

    d_barChartItem = new QwtPlotBarChart( "Bar Chart " );
    d_barChartItem->setLayoutPolicy( QwtPlotBarChart::AutoAdjustSamples );
    d_barChartItem->setSpacing( 10 );
    d_barChartItem->attach( this );

    populate();
    setOrientation( 0 );

    setAutoReplot( true );
}

void BarChart::populate()
{
    QVector< QVector<double> > series;

    for ( int i = 0; i < 10; i++ )
    {
        double sign = 1.0;
#if 0
        if ( i % 3 == 0 )
            sign = -1.0;
#endif

        QVector<double> values;
        for ( int j = 0; j < 3; j++ )
            values += sign * ( 1 + qrand() % 9 );

        series += values;
#if 0
        qDebug() << i << values;
#endif
    }

    d_barChartItem->setSamples( series );
}

void BarChart::setMode( int mode )
{
    if ( mode == 0 )
    {
        d_barChartItem->setStyle( QwtPlotBarChart::Stacked );
    }
    else
    {
        d_barChartItem->setStyle( QwtPlotBarChart::Grouped );
    }
}

void BarChart::setOrientation( int orientation )
{
    QwtPlot::Axis axis1, axis2;

    if ( orientation == 0 )
    {
        axis1 = QwtPlot::xBottom;
        axis2 = QwtPlot::yLeft;

        d_barChartItem->setOrientation( Qt::Vertical );
    }
    else
    {
        axis1 = QwtPlot::yLeft;
        axis2 = QwtPlot::xBottom;

        d_barChartItem->setOrientation( Qt::Horizontal );
    }

    setAxisScale( axis1, 0, d_barChartItem->dataSize() - 1, 1.0 );
    setAxisAutoScale( axis2 );

    QwtScaleDraw *scaleDraw1 = axisScaleDraw( axis1 );
    scaleDraw1->enableComponent( QwtScaleDraw::Backbone, false );
    scaleDraw1->enableComponent( QwtScaleDraw::Ticks, false );

    QwtScaleDraw *scaleDraw2 = axisScaleDraw( axis2 );
    scaleDraw2->enableComponent( QwtScaleDraw::Backbone, true );
    scaleDraw2->enableComponent( QwtScaleDraw::Ticks, true );

    plotLayout()->setCanvasMargin( 0 );
    updateMargins();

    replot();
}

bool BarChart::eventFilter( QObject *object, QEvent *event )
{
    if ( object == canvas() )
    {
        if ( event->type() == QEvent::Resize )
            updateMargins();
    }

    return QwtPlot::eventFilter( object, event );
}

void BarChart::updateMargins()
{
    const int numSamples = d_barChartItem->dataSize();

    if ( d_barChartItem->layoutPolicy() 
        == QwtPlotBarChart::AutoAdjustSamples )
    {
        const int off = 0.5 * d_barChartItem->spacing() - 2;

        if ( d_barChartItem->orientation() == Qt::Vertical )
        {
            const int margin = qCeil( 0.5 * canvas()->width() / numSamples );
            plotLayout()->setCanvasMargin( margin - off, QwtPlot::yLeft );
            plotLayout()->setCanvasMargin( margin - off, QwtPlot::yRight );
        }
        else
        {
            const int margin = qCeil( 0.5 * canvas()->height() / numSamples );
            plotLayout()->setCanvasMargin( margin - off, QwtPlot::xTop );
            plotLayout()->setCanvasMargin( margin - off, QwtPlot::xBottom );
        }
    }
    updateLayout();
}

void BarChart::exportChart()
{
#ifndef QT_NO_PRINTER
    QString fileName = "barchart.pdf";
#else
    QString fileName = "barchart.png";
#endif

#ifndef QT_NO_FILEDIALOG
    const QList<QByteArray> imageFormats =
        QImageWriter::supportedImageFormats();

    QStringList filter;
    filter += "PDF Documents (*.pdf)";
#ifndef QWT_NO_SVG
    filter += "SVG Documents (*.svg)";
#endif
    filter += "Postscript Documents (*.ps)";

    if ( imageFormats.size() > 0 )
    {
        QString imageFilter( "Images (" );
        for ( int i = 0; i < imageFormats.size(); i++ )
        {
            if ( i > 0 )
                imageFilter += " ";
            imageFilter += "*.";
            imageFilter += imageFormats[i];
        }
        imageFilter += ")";

        filter += imageFilter;
    }

    fileName = QFileDialog::getSaveFileName(
        this, "Export File Name", fileName,
        filter.join( ";;" ), NULL, QFileDialog::DontConfirmOverwrite );
#endif
    if ( !fileName.isEmpty() )
    {
        QwtPlotRenderer renderer;
        renderer.renderDocument( this, fileName, QSizeF( 300, 200 ), 85 );
    }
}
