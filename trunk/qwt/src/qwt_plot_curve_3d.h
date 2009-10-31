/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_CURVE_3D_H
#define QWT_PLOT_CURVE_3D_H

#include "qwt_global.h"
#include "qwt_plot_seriesitem.h"
#include "qwt_series_data.h"

class QwtSymbol;
class QwtColorMap;

class QWT_EXPORT QwtPlotCurve3D: public QwtPlotSeriesItem<QwtDoublePoint3D>
{
public:
    enum CurveStyle
    {
        Dots,
        Symbols
    };

    enum PaintAttribute
    {
        ClipPoints = 1
    };

    explicit QwtPlotCurve3D(const QString &title = QString::null);
    explicit QwtPlotCurve3D(const QwtText &title);

    virtual ~QwtPlotCurve3D();

    virtual int rtti() const;

    void setPaintAttribute(PaintAttribute, bool on = true);
    bool testPaintAttribute(PaintAttribute) const;

    void setSamples(const QwtArray<QwtDoublePoint3D> &);

    void setColorMap(const QwtColorMap &);
    const QwtColorMap &colorMap() const;

	void setColorRange(const QwtDoubleInterval &);
	QwtDoubleInterval & colorRange() const;

    void setStyle(CurveStyle style);
    CurveStyle style() const;

    virtual void drawSeries(QPainter *, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &canvasRect, int from, int to) const;

    virtual void updateLegend(QwtLegend *) const;

protected:
    virtual void drawDots(QPainter *, 
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &canvasRect, int from, int to) const;

    void drawSymbols(QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRect &canvasRect, int from, int to) const;


    virtual QwtSymbol *valueSymbol(const QwtDoublePoint3D &) const;
    
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif
