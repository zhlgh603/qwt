/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_SHAPE_ITEM_H
#define QWT_PLOT_SHAPE_ITEM_H

#include "qwt_global.h"
#include "qwt_plot_item.h"
#include <qpainterpath.h>

class QWT_EXPORT QwtPlotShapeItem: public QwtPlotItem
{
public:
    explicit QwtPlotShapeItem( const QString &title = QString::null );
    explicit QwtPlotShapeItem( const QwtText &title );

    virtual ~QwtPlotShapeItem();

    void setRect( const QRectF & );
    void setPolygon( const QPolygonF & );

    void setShape( const QPainterPath & );
    QPainterPath shape() const;

    void setPen( const QPen & );
    QPen pen() const;

    void setBrush( const QBrush & );
    QBrush brush() const;

    void setRenderTolerance( double );
    double renderTolerance() const;

    virtual QRectF boundingRect() const;

    virtual void draw( QPainter *p,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &rect ) const;

    virtual int rtti() const;

protected:
    virtual QPainterPath simplifyPath( const QPainterPath & ) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif
