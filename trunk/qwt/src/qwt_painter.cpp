/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_painter.h"
#include "qwt_math.h"
#include "qwt_clipper.h"
#include "qwt_color_map.h"
#include "qwt_scale_map.h"
#include <qwindowdefs.h>
#include <qwidget.h>
#include <qrect.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpaintdevice.h>
#include <qpixmap.h>
#include <qstyle.h>
#include <qtextdocument.h>
#include <qabstracttextdocumentlayout.h>
#include <qstyleoption.h>
#include <qpaintengine.h>

bool QwtPainter::d_polylineSplitting = true;

static inline bool isClippingNeeded(const QPainter *painter, QRectF &clipRect)
{
    bool doClipping = false;
    const QPaintEngine *pe = painter->paintEngine();
    if ( pe && pe->type() == QPaintEngine::SVG )
    {
        // The SVG paint engine ignores any clipping,

        if ( painter->hasClipping() )
        {
            doClipping = true;
            clipRect = painter->clipRegion().boundingRect();
        }
    }

    return doClipping;
}

/*!
  \brief En/Disable line splitting for the raster paint engine

  The raster paint engine paints polylines of many points
  much faster when they are splitted in smaller chunks.

  \sa polylineSplitting()
*/
void QwtPainter::setPolylineSplitting(bool enable)
{
    d_polylineSplitting = enable;
}

/*!
    Wrapper for QPainter::setClipRect()
*/
void QwtPainter::setClipRect(QPainter *painter, const QRect &rect)
{
    painter->setClipRect(rect);
}

/*!
    Wrapper for QPainter::drawRect()
*/
void QwtPainter::drawRect(QPainter *painter, double x, double y, double w, double h) 
{
    drawRect(painter, QRectF(x, y, w, h));
}

/*!
    Wrapper for QPainter::drawRect()
*/
void QwtPainter::drawRect(QPainter *painter, const QRectF &rect) 
{
    const QRectF r = rect;

    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    if ( deviceClipping )
    {
        if ( !clipRect.intersects(r) )
            return;

        if ( !clipRect.contains(r) )
        {
            fillRect(painter, r & clipRect, painter->brush());

            painter->save();
            painter->setBrush(Qt::NoBrush);
            drawPolyline(painter, QPolygonF(r));
            painter->restore();

            return;
        }
    }

    painter->drawRect(r);
}

/*!
    Wrapper for QPainter::fillRect()
*/
void QwtPainter::fillRect(QPainter *painter, 
    const QRectF &rect, const QBrush &brush)
{
    if ( !rect.isValid() )
        return;

    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    /*
      Performance of Qt4 is horrible for non trivial brushs. Without
      clipping expect minutes or hours for repainting large rects
      (might result from zooming)
    */

    if ( deviceClipping )
        clipRect &= painter->window();
    else
        clipRect = painter->window();

    if ( painter->hasClipping() )
        clipRect &= painter->clipRegion().boundingRect();

    QRectF r = rect;
    if ( deviceClipping )
        r = r.intersect(clipRect);

    if ( r.isValid() )
        painter->fillRect(r, brush);
}

/*!
    Wrapper for QPainter::drawPie()
*/
void QwtPainter::drawPie(QPainter *painter, const QRectF &rect, 
    int a, int alen)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);
    if ( deviceClipping && !clipRect.contains(rect) )
        return;

    painter->drawPie(rect, a, alen);
}

/*!
    Wrapper for QPainter::drawEllipse()
*/
void QwtPainter::drawEllipse(QPainter *painter, const QRectF &rect)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    if ( deviceClipping && !clipRect.contains(rect) )
        return;

    painter->drawEllipse(rect);
}

/*!
    Wrapper for QPainter::drawText()
*/
void QwtPainter::drawText(QPainter *painter, double x, double y, 
        const QString &text)
{
    drawText(painter, QPoint(x, y), text);
}

/*!
    Wrapper for QPainter::drawText()
*/
void QwtPainter::drawText(QPainter *painter, const QPointF &pos, 
        const QString &text)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    if ( deviceClipping && !clipRect.contains(pos) )
        return;

    painter->drawText(pos, text);
}

/*!
    Wrapper for QPainter::drawText()
*/
void QwtPainter::drawText(QPainter *painter, 
    double x, double y, double w, double h, 
    int flags, const QString &text)
{
    drawText(painter, QRectF(x, y, w, h), flags, text);
}

/*!
    Wrapper for QPainter::drawText()
*/
void QwtPainter::drawText(QPainter *painter, const QRectF &rect, 
        int flags, const QString &text)
{
    painter->drawText(rect, flags, text);
}

#ifndef QT_NO_RICHTEXT

/*!
  Wrapper for QSimpleRichText::draw()
*/
void QwtPainter::drawSimpleRichText(QPainter *painter, const QRectF &rect,
    int flags, QTextDocument &text)
{
    const QRectF scaledRect = rect;
    text.setPageSize(QSizeF(scaledRect.width(), QWIDGETSIZE_MAX));

    QAbstractTextDocumentLayout* layout = text.documentLayout();

    const double height = qRound(layout->documentSize().height());
    double y = scaledRect.y();
    if (flags & Qt::AlignBottom)
        y += (scaledRect.height() - height);
    else if (flags & Qt::AlignVCenter)
        y += (scaledRect.height() - height)/2;

    QAbstractTextDocumentLayout::PaintContext context;
    context.palette.setColor(QPalette::Text, painter->pen().color());

    painter->save();

    painter->translate(scaledRect.x(), y);
    layout->draw(painter, context);

    painter->restore();
}

#endif // !QT_NO_RICHTEXT


/*!
  Wrapper for QPainter::drawLine()
*/
void QwtPainter::drawLine(QPainter *painter, double x1, double y1, double x2, double y2)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    if ( deviceClipping && 
        !(clipRect.contains(x1, y1) && clipRect.contains(x2, y2)) )
    {
        QPolygonF polygon;
        polygon += QPoint(x1, y1);
        polygon += QPoint(x2, y2);
        drawPolyline(painter, polygon);
        return;
    }

    painter->drawLine(x1, y1, x2, y2);
}

/*!
  Wrapper for QPainter::drawPolygon()
*/
void QwtPainter::drawPolygon(QPainter *painter, const QPolygonF &polygon)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    QPolygonF cpa = polygon;
    if ( deviceClipping )
    {
#if 0
#ifdef __GNUC__
#warning clipping ignores painter transformations
#endif
#endif
        cpa = QwtClipper::clipPolygonF(clipRect, polygon);
    }
    painter->drawPolygon(cpa);
}

/*!
    Wrapper for QPainter::drawPolyline()
*/
void QwtPainter::drawPolyline(QPainter *painter, const QPolygonF &pa)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    QPolygonF cpa = pa;
    if ( deviceClipping )
        cpa = QwtClipper::clipPolygonF(clipRect, cpa);

    bool doSplit = false;

    if ( d_polylineSplitting )
    {
        const QPaintEngine *pe = painter->paintEngine();
        if ( pe && pe->type() == QPaintEngine::Raster )
        {
            /*
                The raster paint engine seems to use some algo with O(n*n).
                ( Qt 4.3 is better than Qt 4.2, but remains unacceptable)
                To work around this problem, we have to split the polygon into
                smaller pieces.
             */
            doSplit = true;
        }
    }

    if ( doSplit )
    {
        const int numPoints = cpa.size();
        const QPointF *points = cpa.data();

        const int splitSize = 20;
        for ( int i = 0; i < numPoints; i += splitSize )
        {
            const int n = qMin(splitSize + 1, cpa.size() - i);
            painter->drawPolyline(points + i, n);
        }
    }
    else
        painter->drawPolyline(cpa);
}

/*!
    Wrapper for QPainter::drawPoint()
*/
void QwtPainter::drawPoint(QPainter *painter, double x, double y)
{
    QRectF clipRect;
    const bool deviceClipping = isClippingNeeded(painter, clipRect);

    const QPointF pos(x, y);

    if ( deviceClipping && !clipRect.contains(pos) )
        return;

    painter->drawPoint(pos);
}

void QwtPainter::drawColoredArc(QPainter *painter, const QRect &rect, 
    int peak, int arc, int interval, const QColor &c1, const QColor &c2)
{
    int h1, s1, v1;
    int h2, s2, v2;

    c1.getHsv(&h1, &s1, &v1);
    c2.getHsv(&h2, &s2, &v2);
    
    arc /= 2;
    for ( int angle = -arc; angle < arc; angle += interval)
    {
        double ratio;
        if ( angle >= 0 )
            ratio = 1.0 - angle / double(arc);
        else
            ratio = 1.0 + angle / double(arc);
            

        QColor c;
        c.setHsv( h1 + qRound(ratio * (h2 - h1)),
            s1 + qRound(ratio * (s2 - s1)),
            v1 + qRound(ratio * (v2 - v1)) );

        painter->setPen(QPen(c, painter->pen().width()));
        painter->drawArc(rect, (peak + angle) * 16, interval * 16);
    }
}

void QwtPainter::drawFocusRect(QPainter *painter, QWidget *widget)
{
    drawFocusRect(painter, widget, widget->rect());
}

void QwtPainter::drawFocusRect(QPainter *painter, QWidget *widget,
    const QRect &rect)
{
    QStyleOptionFocusRect opt;
    opt.init(widget);
    opt.rect = rect;
    opt.state |= QStyle::State_HasFocus;

    widget->style()->drawPrimitive(QStyle::PE_FrameFocusRect, 
        &opt, painter, widget);
}

//!  Draw a round frame
void QwtPainter::drawRoundFrame(QPainter *painter, const QRect &rect,
    int width, const QPalette &palette, bool sunken)
{

    QColor c0 = palette.color(QPalette::Mid);
    QColor c1, c2;
    if ( sunken )
    {
        c1 = palette.color(QPalette::Dark);
        c2 = palette.color(QPalette::Light);
    }
    else
    {
        c1 = palette.color(QPalette::Light);
        c2 = palette.color(QPalette::Dark);
    }

    painter->setPen(QPen(c0, width));
    painter->drawArc(rect, 0, 360 * 16); // full

    const int peak = 150;
    const int interval = 2;

    if ( c0 != c1 )
        drawColoredArc(painter, rect, peak, 160, interval, c0, c1);
    if ( c0 != c2 )
        drawColoredArc(painter, rect, peak + 180, 120, interval, c0, c2);
}

void QwtPainter::drawColorBar(QPainter *painter,
        const QwtColorMap &colorMap, const QwtDoubleInterval &interval,
        const QwtScaleMap &scaleMap, Qt::Orientation orientation,
        const QRectF &rect)
{
    QVector<QRgb> colorTable;
    if ( colorMap.format() == QwtColorMap::Indexed )
        colorTable = colorMap.colorTable(interval);

    QColor c;

    const QRect devRect = rect.toRect();

    /*
      We paint to a pixmap first to have something scalable for printing
      ( f.e. in a Pdf document )
     */
      
    QPixmap pixmap(devRect.size());
    QPainter pmPainter(&pixmap);
    pmPainter.translate(-devRect.x(), -devRect.y());

    if ( orientation == Qt::Horizontal )
    {
        QwtScaleMap sMap = scaleMap;
        sMap.setPaintInterval(devRect.left(), devRect.right());

        for ( int x = devRect.left(); x <= devRect.right(); x++ )
        {
            const double value = sMap.invTransform(x);

            if ( colorMap.format() == QwtColorMap::RGB )
                c.setRgb(colorMap.rgb(interval, value));
            else
                c = colorTable[colorMap.colorIndex(interval, value)];

            pmPainter.setPen(c);
            pmPainter.drawLine(x, devRect.top(), x, devRect.bottom());
        }
    }
    else // Vertical
    {
        QwtScaleMap sMap = scaleMap;
        sMap.setPaintInterval(devRect.bottom(), devRect.top());

        for ( int y = devRect.top(); y <= devRect.bottom(); y++ )
        {
            const double value = sMap.invTransform(y);

            if ( colorMap.format() == QwtColorMap::RGB )
                c.setRgb(colorMap.rgb(interval, value));
            else
                c = colorTable[colorMap.colorIndex(interval, value)];

            pmPainter.setPen(c);
            pmPainter.drawLine(devRect.left(), y, devRect.right(), y);
        }
    }
    pmPainter.end();
    painter->drawPixmap(devRect, pixmap);
}
