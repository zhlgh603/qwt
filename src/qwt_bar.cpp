/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include <qpainter.h>
#include "qwt_math.h"
#include "qwt_bar.h"
#include "qwt_painter.h"

class QwtBar::PrivateData
{
public:
    PrivateData():
        style(QwtBar::NoBar),
        width(5)
    {
    }

    QwtBar::Style style;
    int width;

    QPen pen;
    QBrush brush;
};

QwtBar::QwtBar(Style style) 
{
    d_data = new PrivateData();
    d_data->style = style;
}

QwtBar::~QwtBar()
{
    delete d_data;
}

QwtBar *QwtBar::clone() const
{
    QwtBar *other = new QwtBar;
    *other->d_data = *d_data;

    return other;
}

//! == operator
bool QwtBar::operator==(const QwtBar &other) const
{
    return d_data->style == other.d_data->style &&
        d_data->width == other.d_data->width &&
        d_data->pen == other.d_data->pen &&
        d_data->brush == other.d_data->brush;
}

//! != operator
bool QwtBar::operator!=(const QwtBar &other) const
{
    return !(*this == other);
}

void QwtBar::setStyle(Style style)
{
    d_data->style = style;
}

QwtBar::Style QwtBar::style() const
{
    return d_data->style;
}

void QwtBar::setWidth(int width)
{
    d_data->width = width;
}

int QwtBar::width() const
{
    return d_data->width;
}

void QwtBar::setBrush(const QBrush &brush)
{
    d_data->brush = brush;
}

const QBrush& QwtBar::brush() const
{
    return d_data->brush;
}

void QwtBar::setPen(const QPen &pen)
{
    d_data->pen = pen;
}

const QPen& QwtBar::pen() const
{
    return d_data->pen;
}

void QwtBar::draw(QPainter *painter, Qt::Orientation orientation, 
    const QRect& rect) const
{
    switch(d_data->style)
    {
        case QwtBar::IntervalBar:
        {
            const int pw = qwtMax(painter->pen().width(), 1);

            if ( orientation == Qt::Vertical )
            {
                const int x = rect.center().x();
                QwtPainter::drawLine(painter, x, rect.top(), 
                    x, rect.bottom() );

                if ( rect.width() > pw )
                {
                    QwtPainter::drawLine(painter, 
                        rect.bottomLeft(), rect.bottomRight());
                    QwtPainter::drawLine(painter, 
                        rect.topLeft(), rect.topRight());
                }
            }
            else
            {
                const int y = rect.center().y();
                QwtPainter::drawLine(painter, rect.left(), y, 
                    rect.right(), y);

                if ( rect.width() > pw )
                {
                    QwtPainter::drawLine(painter, 
                        rect.bottomLeft(), rect.topLeft());
                    QwtPainter::drawLine(painter, 
                        rect.bottomRight(), rect.topRight());
                }
            }

            break;
        }
        case QwtBar::Box:
        {
            break;
        }
        default:;
    }
}
