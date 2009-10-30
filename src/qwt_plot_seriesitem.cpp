/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_seriesitem.h"

class QwtPlotAbstractSeriesItem::PrivateData
{
public:
    PrivateData():
        orientation(Qt::Vertical)
    {
    }

    Qt::Orientation orientation;
};

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotAbstractSeriesItem::QwtPlotAbstractSeriesItem(const QwtText &title):
    QwtPlotItem(title)
{
    d_data = new PrivateData();
}

/*!
  Constructor
  \param title Title of the curve   
*/
QwtPlotAbstractSeriesItem::QwtPlotAbstractSeriesItem(const QString &title):
    QwtPlotItem(QwtText(title))
{
    d_data = new PrivateData();
}

//! Destructor
QwtPlotAbstractSeriesItem::~QwtPlotAbstractSeriesItem()
{
    delete d_data;
}

void QwtPlotAbstractSeriesItem::setOrientation(Qt::Orientation orientation)
{
    if ( d_data->orientation != orientation )
    {
        d_data->orientation = orientation;
        itemChanged();
    }
}

Qt::Orientation QwtPlotAbstractSeriesItem::orientation() const
{ 
    return d_data->orientation;
}   

/*!
  \brief Draw the complete series

  \param painter Painter
  \param xMap Maps x-values into pixel coordinates.
  \param yMap Maps y-values into pixel coordinates.
*/
void QwtPlotAbstractSeriesItem::draw(QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &canvasRect) const
{
    drawSeries(painter, xMap, yMap, canvasRect, 0, -1);
}

