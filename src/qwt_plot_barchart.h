/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_PLOT_BAR_CHART_H
#define QWT_PLOT_BAR_CHART_H

#include "qwt_global.h"
#include "qwt_plot_abstract_barchart.h"
#include "qwt_series_data.h"

class QwtColumnRect;
class QwtColumnSymbol;

/*!
  \brief QwtPlotBarChart displays a series of a values as bars.

   Each bar rendered by a QwtColumnSymbol. The symbol for drawing
   a value is created by symbol() that needs to be overloaded
   to customize the bar for a specific value.
 */
class QWT_EXPORT QwtPlotBarChart:
    public QwtPlotAbstractBarChart, public QwtSeriesStore<QPointF>
{
public:
	enum LegendMode
	{
		LegendChartTitle,
		LegendBarTitles
	};

    explicit QwtPlotBarChart( const QString &title = QString::null );
    explicit QwtPlotBarChart( const QwtText &title );

    virtual ~QwtPlotBarChart();

    virtual int rtti() const;

    void setSamples( const QVector<QPointF> & );
    void setSamples( const QVector<double> & );

    void setSymbol( QwtColumnSymbol * );
    const QwtColumnSymbol *symbol() const;

	void setLegendMode( LegendMode );
	LegendMode legendMode() const;

    virtual void drawSeries( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, int from, int to ) const;

    virtual QRectF boundingRect() const;

    virtual QwtColumnSymbol *specialSymbol( 
        int sampleIndex, const QPointF& ) const;

    virtual QwtText barTitle( int sampleIndex ) const;

protected:
    virtual void drawSample( QPainter *painter,
        const QwtScaleMap &xMap, const QwtScaleMap &yMap,
        const QRectF &canvasRect, const QwtInterval &boundingInterval,
        int index, const QPointF& sample ) const;

    virtual void drawBar( QPainter *,
        int sampleIndex, const QPointF& point, 
        const QwtColumnRect & ) const;

	QList<QwtLegendData> legendData() const;
	QwtGraphic legendIcon( int index, const QSizeF & ) const;

private:
    void init();

    class PrivateData;
    PrivateData *d_data;
};

#endif
