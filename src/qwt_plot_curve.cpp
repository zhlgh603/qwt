/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include <qpixmap.h>
#include <qbitarray.h>
#include "qwt_global.h"
#include "qwt_math.h"
#include "qwt_clipper.h"
#include "qwt_painter.h"
#include "qwt_legend.h"
#include "qwt_legend_item.h"
#include "qwt_scale_map.h"
#include "qwt_double_rect.h"
#include "qwt_plot.h"
#include "qwt_plot_canvas.h"
#include "qwt_curve_fitter.h"
#include "qwt_symbol.h"
#include "qwt_plot_curve.h"

static int verifyRange(int size, int &i1, int &i2)
{
    if (size < 1) 
        return 0;

    i1 = qwtLim(i1, 0, size-1);
    i2 = qwtLim(i2, 0, size-1);

    if ( i1 > i2 )
        qSwap(i1, i2);

    return (i2 - i1 + 1);
}

class QwtPlotCurve::PrivateData
{
public:
    class PixelMatrix: private QBitArray
    {
    public:
        PixelMatrix(const QRect& rect):
            QBitArray(rect.width() * rect.height()),
            _rect(rect)
        {
            fill(false);
        }

        inline bool testPixel(const QPoint& pos)
        {
            if ( !_rect.contains(pos) )
                return false;

            const int idx = _rect.width() * (pos.y() - _rect.y()) + 
                (pos.x() - _rect.x());

            const bool marked = testBit(idx);
            if ( !marked )
                setBit(idx, true);

            return !marked;
        }

    private:
        QRect _rect;
    };

    PrivateData():
        style(QwtPlotCurve::Lines),
        reference(0.0),
        attributes(0),
        paintAttributes(0)
    {
        symbol = new QwtSymbol();
        pen = QPen(Qt::black);
        curveFitter = new QwtSplineCurveFitter;
    }

    ~PrivateData()
    {
        delete symbol;
        delete curveFitter;
    }

    QwtPlotCurve::CurveStyle style;
    double reference;

    QwtSymbol *symbol;
    QwtCurveFitter *curveFitter;

    QPen pen;
    QBrush brush;

    int attributes;
    int paintAttributes;
};

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotCurve::QwtPlotCurve(const QwtText &title):
    QwtPlotSeriesItem<QwtDoublePoint>(title)
{
    init();
}

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotCurve::QwtPlotCurve(const QString &title):
    QwtPlotSeriesItem<QwtDoublePoint>(QwtText(title))
{
    init();
}

//! Destructor
QwtPlotCurve::~QwtPlotCurve()
{
    delete d_data;
}

/*!
  \brief Initialize data members
*/
void QwtPlotCurve::init()
{
    setItemAttribute(QwtPlotItem::Legend);
    setItemAttribute(QwtPlotItem::AutoScale);

    d_data = new PrivateData;
    d_series = new QwtPointSeriesData();

    setZ(20.0);
}

//! \return QwtPlotItem::Rtti_PlotCurve
int QwtPlotCurve::rtti() const
{
    return QwtPlotItem::Rtti_PlotCurve;
}

/*!
  Specify an attribute how to draw the curve

  \param attribute Paint attribute
  \param on On/Off
  /sa PaintAttribute, testPaintAttribute()
*/
void QwtPlotCurve::setPaintAttribute(PaintAttribute attribute, bool on)
{
    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;
}

/*!
    \brief Return the current paint attributes
    \sa PaintAttribute, setPaintAttribute()
*/
bool QwtPlotCurve::testPaintAttribute(PaintAttribute attribute) const
{
    return (d_data->paintAttributes & attribute);
}

/*!
  Set the curve's drawing style

  \param style Curve style
  \sa CurveStyle, style()
*/
void QwtPlotCurve::setStyle(CurveStyle style)
{
    if ( style != d_data->style )
    {
        d_data->style = style;
        itemChanged();
    }
}

/*!
    Return the current style
    \sa CurveStyle, setStyle()
*/
QwtPlotCurve::CurveStyle QwtPlotCurve::style() const 
{ 
    return d_data->style; 
}

/*!
  \brief Assign a symbol
  \param symbol Symbol
  \sa symbol()
*/
void QwtPlotCurve::setSymbol(const QwtSymbol &symbol )
{
    delete d_data->symbol;
    d_data->symbol = symbol.clone();
    itemChanged();
}

/*!
    \brief Return the current symbol
    \sa setSymbol()
*/
const QwtSymbol &QwtPlotCurve::symbol() const 
{ 
    return *d_data->symbol; 
}

/*!
  Assign a pen

  The width of non cosmetic pens is scaled according to the resolution
  of the paint device.

  \param pen New pen
  \sa pen(), brush(), QwtPainter::scaledPen()
*/
void QwtPlotCurve::setPen(const QPen &pen)
{
    if ( pen != d_data->pen )
    {
        d_data->pen = pen;
        itemChanged();
    }
}

/*!
    \brief Return the pen used to draw the lines
    \sa setPen(), brush()
*/
const QPen& QwtPlotCurve::pen() const 
{ 
    return d_data->pen; 
}

/*!
  \brief Assign a brush. 

   In case of brush.style() != QBrush::NoBrush 
   and style() != QwtPlotCurve::Sticks
   the area between the curve and the baseline will be filled.

   In case !brush.color().isValid() the area will be filled by
   pen.color(). The fill algorithm simply connects the first and the
   last curve point to the baseline. So the curve data has to be sorted 
   (ascending or descending). 

  \param brush New brush
  \sa brush(), setBaseline(), baseline()
*/
void QwtPlotCurve::setBrush(const QBrush &brush)
{
    if ( brush != d_data->brush )
    {
        d_data->brush = brush;
        itemChanged();
    }
}

/*!
  \brief Return the brush used to fill the area between lines and the baseline
  \sa setBrush(), setBaseline(), baseline()
*/
const QBrush& QwtPlotCurve::brush() const 
{
    return d_data->brush;
}

/*!
  \brief Draw an interval of the curve
  \param painter Painter
  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.
  \param canvasRect Contents rect of the canvas
  \param from Index of the first point to be painted
  \param to Index of the last point to be painted. If to < 0 the 
         curve will be painted to its last point.

  \sa drawCurve(), drawSymbols(),
*/
void QwtPlotCurve::drawSeries(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    const QRect &, int from, int to) const
{
    if ( !painter || dataSize() <= 0 )
        return;

    if (to < 0)
        to = dataSize() - 1;

    if ( verifyRange(dataSize(), from, to) > 0 )
    {
        painter->save();
        painter->setPen(QwtPainter::scaledPen(d_data->pen));

        /*
          Qt 4.0.0 is slow when drawing lines, but it's even 
          slower when the painter has a brush. So we don't
          set the brush before we really need it.
         */

        drawCurve(painter, d_data->style, xMap, yMap, from, to);
        painter->restore();

        if (d_data->symbol->style() != QwtSymbol::NoSymbol)
        {
            painter->save();
            drawSymbols(painter, *d_data->symbol, xMap, yMap, from, to);
            painter->restore();
        }
    }
}

/*!
  \brief Draw the line part (without symbols) of a curve interval. 
  \param painter Painter
  \param style curve style, see QwtPlotCurve::CurveStyle
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted
  \sa draw(), drawDots(), drawLines(), drawSteps(), drawSticks()
*/

void QwtPlotCurve::drawCurve(QPainter *painter, int style,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    switch (style)
    {
        case Lines:
            if ( testCurveAttribute(Fitted) )
            {
                // we always need the complete 
                // curve for fitting
                from = 0;
                to = dataSize() - 1;
            }
            drawLines(painter, xMap, yMap, from, to);
            break;
        case Sticks:
            drawSticks(painter, xMap, yMap, from, to);
            break;
        case Steps:
            drawSteps(painter, xMap, yMap, from, to);
            break;
        case Dots:
            drawDots(painter, xMap, yMap, from, to);
            break;
        case NoCurve:
        default:
            break;
    }
}

/*!
  \brief Draw lines

  If the CurveAttribute Fitted is enabled a QwtCurveFitter tries
  to interpolate/smooth the curve, before it is painted.

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa setCurveAttribute(), setCurveFitter(), draw(), 
      drawLines(), drawDots(), drawSteps(), drawSticks()
*/
void QwtPlotCurve::drawLines(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    int size = to - from + 1;
    if ( size <= 0 )
        return;

    QwtPolygon polyline;
    if ( ( d_data->attributes & Fitted ) && d_data->curveFitter )
    {
        // Transform x and y values to window coordinates
        // to avoid a distinction between linear and
        // logarithmic scales.

#if QT_VERSION < 0x040000
        QwtArray<QwtDoublePoint> points(size);
#else
        QPolygonF points(size);
#endif
        for (int i = from; i <= to; i++)
        {
            const QwtDoublePoint sample = d_series->sample(i);

            QwtDoublePoint &p = points[i];
            p.setX( xMap.xTransform(sample.x()) );
            p.setY( yMap.xTransform(sample.y()) );
        }

        points = d_data->curveFitter->fitCurve(points);
        size = points.size();

        if ( size == 0 )
            return;

        // Round QwtDoublePoints to QPoints
        // When Qwt support for Qt3 has been dropped (Qwt 6.x)
        // we will use a doubles for painting and the following
        // step will be obsolete.

        polyline.resize(size);

        const QwtDoublePoint *p = points.data();
        QPoint *pl = polyline.data();
        if ( d_data->paintAttributes & PaintFiltered )
        {

            QPoint pp(qRound(p[0].x()), qRound(p[0].y()));
            pl[0] = pp;

            int count = 1;
            for (int i = 1; i < size; i++)
            {
                const QPoint pi(qRound(p[i].x()), qRound(p[i].y()));
                if ( pi != pp )
                {
                    pl[count++] = pi;
                    pp = pi;
                }
            }
            if ( count != size )
                polyline.resize(count);
        }
        else
        {
            for ( int i = 0; i < size; i++ )
            {
                pl[i].setX( qRound(p[i].x()) );
                pl[i].setY( qRound(p[i].y()) );
            }
        }
    }
    else
    {
        polyline.resize(size);

        if ( d_data->paintAttributes & PaintFiltered )
        {
            QwtDoublePoint sample = d_series->sample(from);

            QPoint pp( xMap.transform(sample.x()), 
                yMap.transform(sample.y()) );
            polyline.setPoint(0, pp);

            int count = 1;
            for (int i = from + 1; i <= to; i++)
            {
                sample = d_series->sample(i);
                const QPoint pi(xMap.transform(sample.x()), 
                    yMap.transform(sample.y()));
                if ( pi != pp )
                {
                    polyline.setPoint(count, pi);
                    count++;

                    pp = pi;
                }
            }
            if ( count != size )
                polyline.resize(count);
        }
        else
        {
            for (int i = from; i <= to; i++)
            {
                const QwtDoublePoint sample = d_series->sample(i);
                int xi = xMap.transform(sample.x());
                int yi = yMap.transform(sample.y());

                polyline.setPoint(i - from, xi, yi);
            }
        }
    }

    if ( d_data->paintAttributes & ClipPolygons )
        polyline = QwtClipper::clipPolygon(painter->window(), polyline);

    QwtPainter::drawPolyline(painter, polyline);

    if ( d_data->brush.style() != Qt::NoBrush )
        fillCurve(painter, xMap, yMap, polyline);
}

/*!
  Draw sticks

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa draw(), drawCurve(), drawDots(), drawLines(), drawSteps()
*/
void QwtPlotCurve::drawSticks(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    int x0 = xMap.transform(d_data->reference);
    int y0 = yMap.transform(d_data->reference);

    for (int i = from; i <= to; i++)
    {
        const QwtDoublePoint sample = d_series->sample(i);
        const int xi = xMap.transform(sample.x());
        const int yi = yMap.transform(sample.y());

        if (orientation() == Qt::Horizontal)
            QwtPainter::drawLine(painter, x0, yi, xi, yi);
        else
            QwtPainter::drawLine(painter, xi, y0, xi, yi);
    }
}

/*!
  Draw dots

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa draw(), drawCurve(), drawSticks(), drawLines(), drawSteps()
*/
void QwtPlotCurve::drawDots(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    const QRect window = painter->window();
    if ( window.isEmpty() )
        return;

    const bool doFill = d_data->brush.style() != Qt::NoBrush;

    QwtPolygon polyline;
    if ( doFill )
        polyline.resize(to - from + 1);

    if ( to > from && d_data->paintAttributes & PaintFiltered )
    {
        if ( doFill )   
        {
            QwtDoublePoint sample = d_series->sample(from);
            QPoint pp( xMap.transform(sample.x()), 
                yMap.transform(sample.y()) );

            QwtPainter::drawPoint(painter, pp.x(), pp.y());
            polyline.setPoint(0, pp);

            int count = 1;
            for (int i = from + 1; i <= to; i++)
            {
                sample = d_series->sample(i);
                const QPoint pi(xMap.transform(sample.x()), 
                    yMap.transform(sample.y()));
                if ( pi != pp )
                {
                    QwtPainter::drawPoint(painter, pi.x(), pi.y());

                    polyline.setPoint(count, pi);
                    count++;

                    pp = pi;
                }
            }
            if ( int(polyline.size()) != count )
                polyline.resize(count);
        }
        else
        {
            // if we don't need to fill, we can sort out
            // duplicates independent from the order

            PrivateData::PixelMatrix pixelMatrix(window);

            for (int i = from; i <= to; i++)
            {
                const QwtDoublePoint sample = d_series->sample(i);
                const QPoint p( xMap.transform(sample.x()),
                    yMap.transform(sample.y()) );

                if ( pixelMatrix.testPixel(p) )
                    QwtPainter::drawPoint(painter, p.x(), p.y());
            }
        }
    }
    else
    {
        for (int i = from; i <= to; i++)
        {
            const QwtDoublePoint sample = d_series->sample(i);
            const int xi = xMap.transform(sample.x());
            const int yi = yMap.transform(sample.y());
            QwtPainter::drawPoint(painter, xi, yi);

            if ( doFill )
                polyline.setPoint(i - from, xi, yi);
        }
    }

    if ( doFill )
    {
        if ( d_data->paintAttributes & ClipPolygons )
            polyline = QwtClipper::clipPolygon(painter->window(), polyline);

        fillCurve(painter, xMap, yMap, polyline);
    }
}

/*!
  Draw step function

  The direction of the steps depends on Inverted attribute. 

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa CurveAttribute, setCurveAttribute(),
      draw(), drawCurve(), drawDots(), drawLines(), drawSticks()
*/
void QwtPlotCurve::drawSteps(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    QwtPolygon polyline(2 * (to - from) + 1);

    bool inverted = orientation() == Qt::Vertical;
    if ( d_data->attributes & Inverted )
        inverted = !inverted;

    int i,ip;
    for (i = from, ip = 0; i <= to; i++, ip += 2)
    {
        const QwtDoublePoint sample = d_series->sample(i);
        const int xi = xMap.transform(sample.x());
        const int yi = yMap.transform(sample.y());

        if ( ip > 0 )
        {
            if (inverted)
                polyline.setPoint(ip - 1, polyline[ip-2].x(), yi);
            else
                polyline.setPoint(ip - 1, xi, polyline[ip-2].y());
        }

        polyline.setPoint(ip, xi, yi);
    }

    if ( d_data->paintAttributes & ClipPolygons )
        polyline = QwtClipper::clipPolygon(painter->window(), polyline);

    QwtPainter::drawPolyline(painter, polyline);

    if ( d_data->brush.style() != Qt::NoBrush )
        fillCurve(painter, xMap, yMap, polyline);
}


/*!
  Specify an attribute for drawing the curve

  \param attribute Curve attribute
  \param on On/Off

  /sa CurveAttribute, testCurveAttribute(), setCurveFitter()
*/
void QwtPlotCurve::setCurveAttribute(CurveAttribute attribute, bool on)
{
    if ( bool(d_data->attributes & attribute) == on )
        return;

    if ( on )
        d_data->attributes |= attribute;
    else
        d_data->attributes &= ~attribute;

    itemChanged();
}

/*!
    \return true, if attribute is enabled
    \sa CurveAttribute, setCurveAttribute()
*/
bool QwtPlotCurve::testCurveAttribute(CurveAttribute attribute) const 
{ 
    return d_data->attributes & attribute;
}

/*!
  Assign a curve fitter

  The curve fitter "smooths" the curve points, when the Fitted 
  CurveAttribute is set. setCurveFitter(NULL) also disables curve fitting.

  The curve fitter operates on the translated points ( = widget coordinates)
  to be functional for logarithmic scales. Obviously this is less performant
  for fitting algorithms, that reduce the number of points.

  For situations, where curve fitting is used to improve the performance
  of painting huge series of points it might be better to execute the fitter
  on the curve points once and to cache the result in the QwtSeriesData object.

  \param curveFitter() Curve fitter
  \sa Fitted
*/
void QwtPlotCurve::setCurveFitter(QwtCurveFitter *curveFitter)
{
    delete d_data->curveFitter;
    d_data->curveFitter = curveFitter;

    itemChanged();
}

/*!
  Get the curve fitter. If curve fitting is disabled NULL is returned.

  \return Curve fitter
  \sa setCurveFitter(), Fitted
*/
QwtCurveFitter *QwtPlotCurve::curveFitter() const
{
    return d_data->curveFitter;
}

/*! 
  Fill the area between the curve and the baseline with 
  the curve brush

  \param painter Painter
  \param xMap x map
  \param yMap y map
  \param pa Polygon

  \sa setBrush(), setBaseline(), setCurveType()
*/
void QwtPlotCurve::fillCurve(QPainter *painter,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    QwtPolygon &pa) const
{
    if ( d_data->brush.style() == Qt::NoBrush )
        return;

    closePolyline(xMap, yMap, pa);
    if ( pa.count() <= 2 ) // a line can't be filled
        return;

    QBrush b = d_data->brush;
    if ( !b.color().isValid() )
        b.setColor(d_data->pen.color());

    painter->save();

    painter->setPen(QPen(Qt::NoPen));
    painter->setBrush(b);

    QwtPainter::drawPolygon(painter, pa);

    painter->restore();
}

/*!
  \brief Complete a polygon to be a closed polygon 
         including the area between the original polygon
         and the baseline.
  \param xMap X map
  \param yMap Y map
  \param pa Polygon to be completed
*/
void QwtPlotCurve::closePolyline(
    const QwtScaleMap &xMap, const QwtScaleMap &yMap,
    QwtPolygon &pa) const
{
    const int sz = pa.size();
    if ( sz < 2 )
        return;

    pa.resize(sz + 2);

    if ( orientation() == Qt::Vertical )
    {
        pa.setPoint(sz,
            pa.point(sz - 1).x(), yMap.transform(d_data->reference));
        pa.setPoint(pa.size() - 1,
            pa.point(0).x(), yMap.transform(d_data->reference));
    }
    else
    {
        pa.setPoint(sz,
            xMap.transform(d_data->reference), pa.point(sz - 1).y());
        pa.setPoint(sz + 1,
            xMap.transform(d_data->reference), pa.point(0).y());
    }
}

/*!
  \brief Draw symbols
  \param painter Painter
  \param symbol Curve symbol
  \param xMap x map
  \param yMap y map
  \param from index of the first point to be painted
  \param to index of the last point to be painted

  \sa setSymbol(), draw(), drawCurve()
*/
void QwtPlotCurve::drawSymbols(QPainter *painter, const QwtSymbol &symbol,
    const QwtScaleMap &xMap, const QwtScaleMap &yMap, 
    int from, int to) const
{
    painter->setBrush(symbol.brush());
    painter->setPen(symbol.pen());

    QRect rect;
    rect.setSize(QwtPainter::metricsMap().screenToLayout(symbol.size()));

    if ( to > from && d_data->paintAttributes & PaintFiltered )
    {
        const QRect window = painter->window();
        if ( window.isEmpty() )
            return;

        PrivateData::PixelMatrix pixelMatrix(window);

        for (int i = from; i <= to; i++)
        {
            const QwtDoublePoint sample = d_series->sample(i);

            const QPoint pi( xMap.transform(sample.x()),
                yMap.transform(sample.y()) );

            if ( pixelMatrix.testPixel(pi) )
            {
                rect.moveCenter(pi);
                symbol.draw(painter, rect);
            }
        }
    }
    else
    {
        for (int i = from; i <= to; i++)
        {
            const QwtDoublePoint sample = d_series->sample(i);

            const int xi = xMap.transform(sample.x());
            const int yi = yMap.transform(sample.y());

            rect.moveCenter(QPoint(xi, yi));
            symbol.draw(painter, rect);
        }
    }
}

/*!
  \brief Set the value of the baseline

  The baseline is needed for filling the curve with a brush or
  the Sticks drawing style. 
  The default value is 0.0. The interpretation
  of the baseline depends on the CurveType. With QwtPlotCurve::Yfx,
  the baseline is interpreted as a horizontal line at y = baseline(),
  with QwtPlotCurve::Yfy, it is interpreted as a vertical line at
  x = baseline().
  \param reference baseline
  \sa baseline(), setBrush(), setStyle(), setCurveType()
*/
void QwtPlotCurve::setBaseline(double reference)
{
    if ( d_data->reference != reference )
    {
        d_data->reference = reference;
        itemChanged();
    }
}

/*!
    Return the value of the baseline
    \sa setBaseline()
*/
double QwtPlotCurve::baseline() const 
{ 
    return d_data->reference; 
}

/*!
  Find the closest curve point for a specific position

  \param pos Position, where to look for the closest curve point
  \param dist If dist != NULL, closestPoint() returns the distance between
              the position and the clostest curve point
  \return Index of the closest curve point, or -1 if none can be found
          ( f.e when the curve has no points )
  \note closestPoint() implements a dumb algorithm, that iterates
        over all points
*/
int QwtPlotCurve::closestPoint(const QPoint &pos, double *dist) const
{
    if ( plot() == NULL || dataSize() <= 0 )
        return -1;

    const QwtScaleMap xMap = plot()->canvasMap(xAxis());
    const QwtScaleMap yMap = plot()->canvasMap(yAxis());

    int index = -1;
    double dmin = 1.0e10;

    for (int i=0; i < dataSize(); i++)
    {
        const QwtDoublePoint sample = d_series->sample(i);

        const double cx = xMap.xTransform(sample.x()) - pos.x();
        const double cy = yMap.xTransform(sample.y()) - pos.y();

        const double f = qwtSqr(cx) + qwtSqr(cy);
        if (f < dmin)
        {
            index = i;
            dmin = f;
        }
    }
    if ( dist )
        *dist = sqrt(dmin);

    return index;
}

QWidget *QwtPlotCurve::legendItem() const
{
    return new QwtLegendCurveItem;
}

//!  Update the widget that represents the curve on the legend
void QwtPlotCurve::updateLegend(QwtLegend *legend) const
{
    if ( !legend )
        return;

    QwtPlotItem::updateLegend(legend);

    QWidget *widget = legend->find(this);
    if ( !widget || !widget->inherits("QwtLegendItem") )
        return;

    QwtLegendCurveItem *legendItem = (QwtLegendCurveItem *)widget;

#if QT_VERSION < 0x040000
    const bool doUpdate = legendItem->isUpdatesEnabled();
#else
    const bool doUpdate = legendItem->updatesEnabled();
#endif
    legendItem->setUpdatesEnabled(false);

    const int policy = legend->displayPolicy();

    if (policy == QwtLegend::FixedIdentifier)
    {
        int mode = legend->identifierMode();

        if (mode & QwtLegendItem::ShowLine)
            legendItem->setCurvePen(pen());

        if (mode & QwtLegendItem::ShowSymbol)
            legendItem->setSymbol(symbol());

        if (mode & QwtLegendItem::ShowText)
            legendItem->setText(title());
        else
            legendItem->setText(QwtText());

        legendItem->setIdentifierMode(mode);
    }
    else if (policy == QwtLegend::AutoIdentifier)
    {
        int mode = 0;

        if (QwtPlotCurve::NoCurve != style())
        {
            legendItem->setCurvePen(pen());
            mode |= QwtLegendItem::ShowLine;
        }
        if (QwtSymbol::NoSymbol != symbol().style())
        {
            legendItem->setSymbol(symbol());
            mode |= QwtLegendItem::ShowSymbol;
        }
        if ( !title().isEmpty() )
        {
            legendItem->setText(title());
            mode |= QwtLegendItem::ShowText;
        }
        else
        {
            legendItem->setText(QwtText());
        }
        legendItem->setIdentifierMode(mode);
    }

    legendItem->setUpdatesEnabled(doUpdate);
    legendItem->update();
}

/*!
  Initialize data with an array of points (explicitly shared).

  \param data Data
  \sa QwtPolygonFData
*/
void QwtPlotCurve::setSamples(const QwtArray<QwtDoublePoint> &data)
{
    delete d_series;
    d_series = new QwtPointSeriesData(data);
    itemChanged();
}

#ifndef QWT_NO_COMPAT

/*!
  \brief Initialize the data by pointing to memory blocks which are not managed
  by QwtPlotCurve.

  setRawSamples is provided for efficiency. It is important to keep the pointers
  during the lifetime of the underlying QwtCPointerData class.

  \param xData pointer to x data
  \param yData pointer to y data
  \param size size of x and y

  \sa QwtCPointerData::setSamples()
*/
void QwtPlotCurve::setRawSamples(const double *xData, const double *yData, int size)
{
    delete d_series;
    d_series = new QwtCPointerData(xData, yData, size);
    itemChanged();
}

/*!
  Set data by copying x- and y-values from specified memory blocks.
  Contrary to setRawSamples(), this function makes a 'deep copy' of
  the data.

  \param xData pointer to x values
  \param yData pointer to y values
  \param size size of xData and yData

  \sa QwtCPointerData
*/
void QwtPlotCurve::setSamples(const double *xData, const double *yData, int size)
{
    delete d_series;
    d_series = new QwtPointArrayData(xData, yData, size);
    itemChanged();
}

/*!
  \brief Initialize data with x- and y-arrays (explicitly shared)

  \param xData x data
  \param yData y data

  \sa QwtArrayData
*/
void QwtPlotCurve::setSamples(const QwtArray<double> &xData, 
    const QwtArray<double> &yData)
{
    delete d_series;
    d_series = new QwtPointArrayData(xData, yData);
    itemChanged();
}
#endif // !QWT_NO_COMPAT

