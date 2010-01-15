/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_intervalcurve.h"
#include "qwt_interval_symbol.h"
#include "qwt_scale_map.h"
#include "qwt_painter.h"

#include <qpainter.h>

class QwtPlotIntervalCurve::PrivateData
{
public:
    PrivateData():
        curveStyle(Tube),
        pen(Qt::black),
        brush(Qt::white)
    {
        symbol = new QwtIntervalSymbol();
    }

    ~PrivateData()
    {
        delete symbol;
    }

    CurveStyle curveStyle;
    QwtIntervalSymbol *symbol;

    QPen pen;
    QBrush brush;
};

/*!
  \brief Ctor
  \param title title of the curve   
*/
QwtPlotIntervalCurve::QwtPlotIntervalCurve(const QwtText &title):
    QwtPlotSeriesItem<QwtIntervalSample>(title)
{
    init();
}

/*!
  \brief Ctor
  \param title title of the curve   
*/
QwtPlotIntervalCurve::QwtPlotIntervalCurve(const QString &title):
    QwtPlotSeriesItem<QwtIntervalSample>(QwtText(title))
{
    init();
}

//! Dtor
QwtPlotIntervalCurve::~QwtPlotIntervalCurve()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotIntervalCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend, true);
    setItemAttribute(QwtPlotItem::AutoScale, true);

    d_data = new PrivateData;
    d_series = new QwtIntervalSeriesData();

    setZ(19.0);
}

int QwtPlotIntervalCurve::rtti() const
{
    return QwtPlotIntervalCurve::Rtti_PlotIntervalCurve;
}

void QwtPlotIntervalCurve::setSamples(
    const QVector<QwtIntervalSample> &data)
{
    delete d_series;
    d_series = new QwtIntervalSeriesData(data);
    itemChanged();
}

void QwtPlotIntervalCurve::setCurveStyle(CurveStyle style)
{
    if ( style != d_data->curveStyle )
    {
        d_data->curveStyle = style;
        itemChanged();
    }
}

/*!
    \brief Return the current style
    \sa setStyle()
*/
QwtPlotIntervalCurve::CurveStyle QwtPlotIntervalCurve::curveStyle() const 
{ 
    return d_data->curveStyle; 
}

void QwtPlotIntervalCurve::setSymbol(const QwtIntervalSymbol &symbol)
{
    if ( symbol != *d_data->symbol )
    {
        delete d_data->symbol;
        d_data->symbol = symbol.clone();
        itemChanged();
    }
}

const QwtIntervalSymbol &QwtPlotIntervalCurve::symbol() const 
{ 
    return *d_data->symbol; 
}

/*!
  \brief Assign a pen
  \param p New pen
  \sa pen(), brush()
*/
void QwtPlotIntervalCurve::setPen(const QPen &p)
{
    if ( p != d_data->pen )
    {
        d_data->pen = p;
        itemChanged();
    }
}

/*!
    \brief Return the pen used to draw the lines
    \sa setPen(), brush()
*/
const QPen& QwtPlotIntervalCurve::pen() const 
{ 
    return d_data->pen; 
}

void QwtPlotIntervalCurve::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    {
        d_data->brush = brush;
        itemChanged();
    }
}

const QBrush& QwtPlotIntervalCurve::brush() const 
{
    return d_data->brush;
}

QRectF QwtPlotIntervalCurve::boundingRect() const
{
    QRectF br = QwtPlotSeriesItem<QwtIntervalSample>::boundingRect();
    if ( br.isValid() )
    {
        if ( orientation() == Qt::Vertical )
            br.setRect(br.y(), br.x(), br.height(), br.width());
        else
            br.setRect(br.x(), br.y(), br.width(), br.height());
    }

    return br;
}

void QwtPlotIntervalCurve::drawSeries(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    const QRect &, int from, int to) const
{
    if (to < 0)
        to = dataSize() - 1;

    if ( from < 0 )
        from = 0;

    if ( from >= to )
        return;
        
    switch(d_data->curveStyle)
    {
        case Tube:
            drawTube(painter, xMap, yMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }

    if ( d_data->symbol->style() != QwtIntervalSymbol::NoSymbol )
        drawSymbols(painter, xMap, yMap, from, to);
}

void QwtPlotIntervalCurve::drawTube(QPainter *painter, 
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->save();

    const size_t size = to - from + 1;
    QPolygon points(2 * size);

    for ( uint i = 0; i < size; i++ )
    {
        QPoint &minValue = points[i];
        QPoint &maxValue = points[2 * size - 1 - i];

        const QwtIntervalSample intervalSample = sample(from + i);
        if ( orientation() == Qt::Vertical )
        {
            const int x = xMap.transform(intervalSample.value);
            const int y1 = yMap.transform(intervalSample.interval.minValue());
            const int y2 = yMap.transform(intervalSample.interval.maxValue());

            minValue.setX(x);
            minValue.setY(y1);
            maxValue.setX(x);
            maxValue.setY(y2);
        }
        else
        {
            const int y = yMap.transform(intervalSample.value);
            const int x1 = xMap.transform(intervalSample.interval.minValue());
            const int x2 = xMap.transform(intervalSample.interval.maxValue());

            minValue.setX(x1);
            minValue.setY(y);
            maxValue.setX(x2);
            maxValue.setY(y);
        }
    }

#if 1
    if ( d_data->brush.style() != Qt::NoBrush )
    {
        painter->setPen(QPen(Qt::NoPen));
        painter->setBrush(d_data->brush);
        QwtPainter::drawPolygon(painter, points);
    }

    if ( d_data->pen.style() != Qt::NoPen )
    {
        painter->setPen(d_data->pen);
        painter->setBrush(Qt::NoBrush);

        QPolygon curve;
        curve = points.mid(0, size);
        QwtPainter::drawPolyline(painter, curve);
        curve = points.mid(size, size);
        QwtPainter::drawPolyline(painter, curve);
    }
#else
    painter->setPen(d_data->pen);
    painter->setBrush(d_data->brush);
    QwtPainter::drawPolygon(painter, points);
#endif

    painter->restore();
}

void QwtPlotIntervalCurve::drawSymbols(
    QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setPen(d_data->symbol->pen());
    painter->setBrush(d_data->symbol->brush());

    for ( int i = from; i <= to; i++ )
    {
        const QwtIntervalSample intervalSample = sample(i);

        if ( orientation() == Qt::Vertical )
        {
            const int x = xMap.transform(intervalSample.value);
            const int y1 = yMap.transform(intervalSample.interval.minValue());
            const int y2 = yMap.transform(intervalSample.interval.maxValue());

            d_data->symbol->draw(painter, QPoint(x, y1), QPoint(x, y2));
        }
        else
        {
            const int y = yMap.transform(intervalSample.value);
            const int x1 = xMap.transform(intervalSample.interval.minValue());
            const int x2 = xMap.transform(intervalSample.interval.maxValue());

            d_data->symbol->draw(painter, QPoint(x1, y), QPoint(x2, y));
        }
    }

    painter->restore();
}

void QwtPlotIntervalCurve::drawLegendIdentifier(
    QPainter *painter, const QRect &rect) const
{
    const int dim = qMin(rect.width(), rect.height());

    QSize size(dim, dim);
    size = QwtPainter::metricsMap().screenToLayout(size);

    QRect r(0, 0, size.width(), size.height());
    r.moveCenter(rect.center());

    if (d_data->curveStyle == Tube)
    {
        painter->fillRect(r, d_data->brush);
    }

    if ( d_data->symbol->style() != QwtIntervalSymbol::NoSymbol )
    {
        painter->setPen(d_data->symbol->pen());
        painter->setBrush(d_data->symbol->brush());

        if ( orientation() == Qt::Vertical )
        {
            d_data->symbol->draw(painter,
                QPoint(r.center().x(), r.top()), 
                QPoint(r.center().x(), r.bottom()) );
        }
        else
        {
            d_data->symbol->draw(painter,
                QPoint(r.left(), r.center().y()), 
                QPoint(r.right(), r.center().y()) );
        }
    }
}
