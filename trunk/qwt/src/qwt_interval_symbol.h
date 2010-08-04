/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef QWT_INTERVAL_SYMBOL_H
#define QWT_INTERVAL_SYMBOL_H

#include "qwt_global.h"
#include <qpen.h>
#include <qsize.h>

class QPainter;
class QRect;
class QPointF;

/*! 
  \brief A drawing primitive for displaying an interval like an error bar

  \sa QwtPlotIntervalCurve
*/
class QWT_EXPORT QwtIntervalSymbol
{
public:
    /*!
        Style

        - NoSymbol\n
          No Style. The symbol cannot be drawn.

        - Bar\n
          The symbol displays a line with caps at the beginning/end. 
          The size of the caps depends on the symbol width(). 

        - Box\n
          The symbol displays a plain rectangle using pen() and brush().
          The size of the rectangle depends on the translated interval and 
          the width(),

        - UserSymbol\n
          Styles >= UserSymbol are reserved for derived
          classes of QwtIntervalSymbol that overload draw() with
          additional application specific symbol types.

        \sa setStyle(), style()
     */
    enum Style 
    { 
        NoSymbol = -1, 

        Bar, 
        Box, 

        UserSymbol = 1000 
    };
   
public:
    QwtIntervalSymbol(Style = NoSymbol);
    virtual ~QwtIntervalSymbol();
    
    void setWidth(int);
    int width() const;

    void setBrush(const QBrush& b);
    const QBrush& brush() const;

    void setPen(const QPen &);
    const QPen& pen() const; 

    void setStyle(Style);
    Style style() const;
    
    virtual void draw(QPainter *, 
        const QPointF& from, const QPointF& to) const;

private:
    // Disabled copy constructor and operator=
    QwtIntervalSymbol( const QwtIntervalSymbol & );
    QwtIntervalSymbol &operator=( const QwtIntervalSymbol & );

    class PrivateData;
    PrivateData* d_data;
};

#endif
