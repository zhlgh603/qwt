/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_plot_canvas.h"
#include "qwt_painter.h"
#include "qwt_null_paintdevice.h"
#include "qwt_math.h"
#include "qwt_plot.h"
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpaintengine.h>
#include <qevent.h>
#include <qbitmap.h>
#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

class QwtStyleSheetRecorder: public QwtNullPaintDevice
{
public:
    QwtStyleSheetRecorder( const QSize &size ):
        QwtNullPaintDevice( QPaintEngine::AllFeatures )
    {
        setSize( size );
    }

    virtual void updateState( const QPaintEngineState &state )
    {
        if ( state.state() & QPaintEngine::DirtyPen )
        {
            d_pen = state.pen();
        }
        if ( state.state() & QPaintEngine::DirtyBrush )
        {
            d_brush = state.brush();
        }
        if ( state.state() & QPaintEngine::DirtyBrushOrigin )
        {
            d_origin = state.brushOrigin();
        }
    }

    virtual void drawRects(const QRectF *rects, int count )
    {
        for ( int i = 0; i < count; i++ )
            border.rectList += rects[i];
    }

    virtual void drawPath( const QPainterPath &path )
    {
        const QRectF rect( QPointF( 0.0, 0.0 ) , size() );
        if ( path.controlPointRect().contains( rect.center() ) )
        {
            setCornerRects( path );
            alignCornerRects( rect );

            background.path = path;
            background.brush = d_brush;
            background.origin = d_origin;
        }
        else
        {
            border.pathList += path;
        }
    }

    void setCornerRects( const QPainterPath &path )
    {
        QPointF pos( 0.0, 0.0 );

        for ( int i = 0; i < path.elementCount(); i++ )
        {
            QPainterPath::Element el = path.elementAt(i); 
            switch( el.type )
            {
                case QPainterPath::MoveToElement:
                case QPainterPath::LineToElement:
                {
                    pos.setX( el.x );
                    pos.setY( el.y );
                    break;
                }
                case QPainterPath::CurveToElement:
                {
                    QRectF r( pos, QPointF( el.x, el.y ) );
                    clipRects += r.normalized();

                    pos.setX( el.x );
                    pos.setY( el.y );

                    break;
                }
                case QPainterPath::CurveToDataElement:
                {
                    if ( clipRects.size() > 0 )
                    {
                        QRectF r = clipRects.last();
                        r.setCoords( 
                            qMin( r.left(), el.x ),
                            qMin( r.top(), el.y ),
                            qMax( r.right(), el.x ),
                            qMax( r.bottom(), el.y )
                        );
                        clipRects.last() = r.normalized();
                    }
                    break;
                }
            }
        }
    }

private:
    void alignCornerRects( const QRectF &rect )
    {
        for ( int i = 0; i < clipRects.size(); i++ )
        {
            QRectF &r = clipRects[i];
            if ( r.center().x() < rect.center().x() )
                r.setLeft( rect.left() );
            else
                r.setRight( rect.right() );

            if ( r.center().y() < rect.center().y() )
                r.setTop( rect.top() );
            else
                r.setBottom( rect.bottom() );
        }
    }


public:
    QVector<QRectF> clipRects;

    struct
    {
        QList<QPainterPath> pathList;
        QList<QRectF> rectList;
        QRegion clipRegion;
    } border;

    struct
    {
        QPainterPath path;
        QBrush brush;
        QPointF origin;
    } background;

private:
    QPen d_pen;
    QBrush d_brush;
    QPointF d_origin;
};

static void qwtDrawBackground( QPainter *painter, QWidget *widget )
{
    const QBrush &brush = 
        widget->palette().brush( widget->backgroundRole() );

    if ( brush.style() == Qt::TexturePattern )
    {
        QPixmap pm( widget->size() );
        pm.fill( widget, 0, 0 );
        painter->drawPixmap( 0, 0, pm );
    }
    else if ( brush.gradient() )
    {
        QVector<QRect> rects;

        if ( brush.gradient()->coordinateMode() == QGradient::ObjectBoundingMode )
        {
            rects += widget->rect();
        } 
        else 
        {
            rects = painter->clipRegion().rects();
        }

#if 1
        bool useRaster = false;

        if ( painter->paintEngine()->type() == QPaintEngine::X11 )
        {
            // Qt 4.7.1: gradients on X11 are broken ( subrects + 
            // QGradient::StretchToDeviceMode ) and horrible slow.
            // As workaround we have to use the raster paintengine.
            // Even if the QImage -> QPixmap translation is slow
            // it is three times faster, than using X11 directly

            useRaster = true;
        }
#endif
        if ( useRaster )
        {
            QImage::Format format = QImage::Format_RGB32;

            const QGradientStops stops = brush.gradient()->stops();
            for ( int i = 0; i < stops.size(); i++ )
            {
                if ( stops[i].second.alpha() != 255 )
                {
                    // don't use Format_ARGB32_Premultiplied. It's
                    // recommended by the Qt docs, but QPainter::drawImage()
                    // is horrible slow on X11.

                    format = QImage::Format_ARGB32;
                    break;
                }
            }
            
            QImage image( widget->size(), format );

            QPainter p( &image );
            p.setPen( Qt::NoPen );
            p.setBrush( brush );

            p.drawRects( rects );

            p.end();

            painter->drawImage( 0, 0, image );
        }
        else
        {
            painter->save();

            painter->setPen( Qt::NoPen );
            painter->setBrush( brush );

            painter->drawRects( rects );

            painter->restore();
        }
    }
    else
    {
        painter->save();

        painter->setPen( Qt::NoPen );
        painter->setBrush( brush );

        painter->drawRects( painter->clipRegion().rects() );

        painter->restore();
    }
}

static inline void qwtRevertPath( QPainterPath &path )
{
    if ( path.elementCount() == 4 )
    {
        QPainterPath::Element &el0 = 
            const_cast<QPainterPath::Element &>( path.elementAt(0) );
        QPainterPath::Element &el2 = 
            const_cast<QPainterPath::Element &>( path.elementAt(3) );

        qSwap( el0.x, el2.x );
        qSwap( el0.y, el2.y );
    }
}

static QPainterPath qwtCombinePathList( const QRectF &rect, 
    const QList<QPainterPath> &pathList )
{
    if ( pathList.isEmpty() )
        return QPainterPath();

    QPainterPath ordered[8]; // starting top left

    for ( int i = 0; i < pathList.size(); i++ )
    {
        int index = -1;
        QPainterPath subPath = pathList[i];

        const QRectF br = pathList[i].controlPointRect();
        if ( br.center().x() < rect.center().x() )
        {
            if ( br.center().y() < rect.center().y() )
            {
                if ( qAbs( br.top() - rect.top() ) < 
                    qAbs( br.left() - rect.left() ) )
                {
                    index = 0;
                }
                else
                {
                    index = 7;
                }
            }
            else
            {
                if ( qAbs( br.bottom() - rect.bottom() ) < 
                    qAbs( br.left() - rect.left() ) )
                {
                    index = 5;
                }
                else
                {
                    index = 6;
                }
            }

            if ( subPath.currentPosition().y() > br.center().y() )
                qwtRevertPath( subPath );
        }
        else
        {
            if ( br.center().y() < rect.center().y() )
            {
                if ( qAbs( br.top() - rect.top() ) < 
                    qAbs( br.right() - rect.right() ) )
                {
                    index = 1;
                }
                else
                {
                    index = 2;
                }
            }
            else
            {
                if ( qAbs( br.bottom() - rect.bottom() ) < 
                    qAbs( br.right() - rect.right() ) )
                {
                    index = 4;
                }
                else
                {
                    index = 3;
                }
            }
            if ( subPath.currentPosition().y() < br.center().y() )
                qwtRevertPath( subPath );
        }   
        ordered[index] = subPath;
    }

    QPainterPath path = ordered[0];
    for ( int i = 1; i < 8; i++ )
        path.connectPath( ordered[i] );

    return path.simplified();
}

static inline void qwtDrawStyledBackground( 
    QWidget *w, QPainter *painter )
{
    QStyleOption opt;
    opt.initFrom(w);
    w->style()->drawPrimitive( QStyle::PE_Widget, &opt, painter, w);
}

static QWidget *qwtBackgroundWidget( QWidget *w )
{
    if ( w->parentWidget() == NULL )
        return w;

    if ( w->autoFillBackground() )
    {
        const QBrush brush = w->palette().brush( w->backgroundRole() );
        if ( brush.color().alpha() > 0 )
            return w;
    }

    if ( w->testAttribute( Qt::WA_StyledBackground ) )
    {
        QImage image( 1, 1, QImage::Format_ARGB32 );
        image.fill( Qt::transparent );

        QPainter painter( &image );
        painter.translate( -w->rect().center() );
        qwtDrawStyledBackground( w, &painter );
        painter.end();

        if ( qAlpha( image.pixel( 0, 0 ) ) != 0 )
            return w;
    }

    return qwtBackgroundWidget( w->parentWidget() );
}

static void qwtFillBackground( QPainter *painter, QWidget *widget )
{
    QwtStyleSheetRecorder recorder( widget->size() );

    QPainter p( &recorder );
    qwtDrawStyledBackground( widget, &p );
    p.end();

    QRegion clipRegion;
    if ( painter->hasClipping() )
        clipRegion = painter->transform().map( painter->clipRegion() );
    else
        clipRegion = widget->contentsRect();

    QWidget *bgWidget = NULL;

    QVector<QRectF> fillRects;
    if ( recorder.background.brush.isOpaque() )
    {
        fillRects = recorder.clipRects;
    }
    else
    {
        fillRects += widget->rect();
    }

    for ( int i = 0; i < fillRects.size(); i++ )
    {
        const QRect rect = fillRects[i].toAlignedRect();
        if ( clipRegion.intersects( rect ) )
        {
            if ( bgWidget == NULL )
            {
                // Try to find out which widget fills
                // the unfilled areas of the styled background

                bgWidget = qwtBackgroundWidget( widget->parentWidget() );
            }

            QPixmap pm( rect.size() );
            pm.fill( bgWidget, widget->mapTo( bgWidget, rect.topLeft() ) );
            painter->drawPixmap( rect, pm );
        }
    }
}

class QwtPlotCanvas::PrivateData
{
public:
    PrivateData():
        focusIndicator( NoFocusIndicator ),
        paintAttributes( 0 ),
        backingStore( NULL )
    {
        styleSheet.hasBorder = false;
    }

    ~PrivateData()
    {
        delete backingStore;
    }

    FocusIndicator focusIndicator;
    int paintAttributes;
    QPixmap *backingStore;

    struct
    {
        bool hasBorder;
        QPainterPath borderPath;
        QVector<QRectF> cornerRects;

        struct
        {
            QPainterPath path;
            QBrush brush;
            QPointF origin;
        } background;

    } styleSheet;

};

//! Sets a cross cursor, enables QwtPlotCanvas::BackingStore

QwtPlotCanvas::QwtPlotCanvas( QwtPlot *plot ):
    QFrame( plot )
{
    d_data = new PrivateData;

#ifndef QT_NO_CURSOR
    setCursor( Qt::CrossCursor );
#endif

    setAutoFillBackground( true );
    setPaintAttribute( QwtPlotCanvas::BackingStore, true );
    setPaintAttribute( QwtPlotCanvas::Opaque, true );
    setPaintAttribute( QwtPlotCanvas::HackStyledBackground, true );
}

//! Destructor
QwtPlotCanvas::~QwtPlotCanvas()
{
    delete d_data;
}

//! Return parent plot widget
QwtPlot *QwtPlotCanvas::plot()
{
    return qobject_cast<QwtPlot *>( parentWidget() );
}

//! Return parent plot widget
const QwtPlot *QwtPlotCanvas::plot() const
{
    return qobject_cast<const QwtPlot *>( parentWidget() );
}

/*!
  \brief Changing the paint attributes

  \param attribute Paint attribute
  \param on On/Off

  \sa testPaintAttribute(), backingStore()
*/
void QwtPlotCanvas::setPaintAttribute( PaintAttribute attribute, bool on )
{
    if ( bool( d_data->paintAttributes & attribute ) == on )
        return;

    if ( on )
        d_data->paintAttributes |= attribute;
    else
        d_data->paintAttributes &= ~attribute;

    switch ( attribute )
    {
        case BackingStore:
        {
            if ( on )
            {
                if ( d_data->backingStore == NULL )
                    d_data->backingStore = new QPixmap();

                if ( isVisible() )
                {
                    *d_data->backingStore = 
                        QPixmap::grabWidget( this, rect() );
                }
            }
            else
            {
                delete d_data->backingStore;
                d_data->backingStore = NULL;
            }
            break;
        }
        case Opaque:
        {
            if ( on )
                setAttribute( Qt::WA_OpaquePaintEvent, true );

            break;
        }
        case HackStyledBackground:
        {
            break;
        }
    }
}

/*!
  Test wether a paint attribute is enabled

  \param attribute Paint attribute
  \return true if the attribute is enabled
  \sa setPaintAttribute()
*/
bool QwtPlotCanvas::testPaintAttribute( PaintAttribute attribute ) const
{
    return ( d_data->paintAttributes & attribute ) != 0;
}

//! Return the backing store, might be null
const QPixmap *QwtPlotCanvas::backingStore() const
{
    return d_data->backingStore;
}

//! Invalidate the internal backing store
void QwtPlotCanvas::invalidateBackingStore()
{
    if ( d_data->backingStore )
        *d_data->backingStore = QPixmap();
}

/*!
  Set the focus indicator

  \sa FocusIndicator, focusIndicator()
*/
void QwtPlotCanvas::setFocusIndicator( FocusIndicator focusIndicator )
{
    d_data->focusIndicator = focusIndicator;
}

/*!
  \return Focus indicator

  \sa FocusIndicator, setFocusIndicator()
*/
QwtPlotCanvas::FocusIndicator QwtPlotCanvas::focusIndicator() const
{
    return d_data->focusIndicator;
}

bool QwtPlotCanvas::event( QEvent *event )
{
    if ( event->type() == QEvent::PolishRequest ) 
    {
        if ( testPaintAttribute( QwtPlotCanvas::Opaque ) )
        {
            // Setting a style sheet changes the 
            // Qt::WA_OpaquePaintEvent attribute, but we insist
            // on painting the background.
            
            setAttribute( Qt::WA_OpaquePaintEvent, true );
        }
    }

    if ( event->type() == QEvent::PolishRequest || 
        event->type() == QEvent::StyleChange )
    {
        updateStyleSheetInfo();
    }

    return QFrame::event( event );
}

/*!
  Paint event
  \param event Paint event
*/
void QwtPlotCanvas::paintEvent( QPaintEvent *event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) &&
        d_data->backingStore != NULL )
    {
        QPixmap &bs = *d_data->backingStore;
        if ( bs.size() != size() )
        {
            bs = QPixmap( size() );

#ifdef Q_WS_X11
            if ( bs.x11Info().screen() != x11Info().screen() )
                bs.x11SetScreen( x11Info().screen() );
#endif

            if ( testAttribute(Qt::WA_StyledBackground) )
            {
                QPainter p( &bs );
                qwtFillBackground( &p, this );
                drawCanvas( &p, true );
            }
            else
            {
                bs.fill( this, 0, 0 );

                QPainter p( &bs );
                drawCanvas( &p, false );

                if ( frameWidth() > 0 )
                    drawFrame( &p );
            }
        }

        painter.drawPixmap( 0, 0, *d_data->backingStore );
    }
    else
    {
        if ( testAttribute(Qt::WA_StyledBackground ) )
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                qwtFillBackground( &painter, this );
                drawCanvas( &painter, true );
            }
            else
            {
                drawCanvas( &painter, false );
            }
        }
        else
        {
            if ( testAttribute( Qt::WA_OpaquePaintEvent ) )
            {
                if ( autoFillBackground() )
                    qwtDrawBackground( &painter, this );
            }

            drawCanvas( &painter, false );

            if ( ( frameWidth() > 0 ) 
                && !contentsRect().contains( event->rect() ) )
            {
                drawFrame( &painter );
            }
        }
    }

    if ( hasFocus() && focusIndicator() == CanvasFocusIndicator )
        drawFocusIndicator( &painter );
}

void QwtPlotCanvas::drawCanvas( QPainter *painter, bool styled ) 
{
    bool hackStyledBackground = false;

    if ( styled && testPaintAttribute( HackStyledBackground ) )
    {
        // Antialiasing rounded borders is done by
        // inserting pixels with colors between the 
        // border color and the color on the canvas,
        // When the border is painted before the plot items
        // these colors are interpolated for the canvas
        // and the plot items need to be clipped excluding
        // the anialiased pixels. In situations, where
        // the plot items fill the area at the rounded
        // borders this is noticeable.
        // The only way to avoid these annoying "artefacts"
        // is to paint the border on top of the plot items.

        if ( d_data->styleSheet.hasBorder &&
            !d_data->styleSheet.borderPath.isEmpty() )
        {
            // We have a border with at least one rounded corner
            hackStyledBackground = true;
        }

        if ( hackStyledBackground )
        {
            // paint background without border

            painter->save();

            painter->setPen( Qt::NoPen );
            painter->setBrush( d_data->styleSheet.background.brush ); 
            painter->setBrushOrigin( d_data->styleSheet.background.origin );
#if 0
            painter->drawPath( d_data->styleSheet.borderPath );
#else
            painter->setClipPath( d_data->styleSheet.borderPath );
            painter->drawRect( contentsRect() );
#endif

            painter->restore();
        }
        else
        {
            qwtDrawStyledBackground( this, painter );
        }
    }

    painter->save();

    if ( !d_data->styleSheet.borderPath.isEmpty() )
    {
        painter->setClipPath( 
            d_data->styleSheet.borderPath, Qt::IntersectClip );
    }
    else
    {
        painter->setClipRect( contentsRect(), Qt::IntersectClip );
    }

    plot()->drawCanvas( painter );

    painter->restore();

    if ( hackStyledBackground )
    {
        // Now paint the border on top
        QStyleOptionFrame opt;
        opt.initFrom(this);
        style()->drawPrimitive( QStyle::PE_Frame, &opt, painter, this);
    }
}

/*!
  Resize event
  \param event Resize event
*/
void QwtPlotCanvas::resizeEvent( QResizeEvent *event )
{
    QFrame::resizeEvent( event );
    updateStyleSheetInfo();
}

/*!
  Draw the focus indication
  \param painter Painter
*/
void QwtPlotCanvas::drawFocusIndicator( QPainter *painter )
{
    const int margin = 1;

    QRect focusRect = contentsRect();
    focusRect.setRect( focusRect.x() + margin, focusRect.y() + margin,
        focusRect.width() - 2 * margin, focusRect.height() - 2 * margin );

    QwtPainter::drawFocusRect( painter, this, focusRect );
}

/*!
   Invalidate the paint cache and repaint the canvas
   \sa invalidatePaintCache()
*/
void QwtPlotCanvas::replot()
{
    invalidateBackingStore();
    repaint( contentsRect() );
}

void QwtPlotCanvas::updateStyleSheetInfo()
{
    if ( !testAttribute(Qt::WA_StyledBackground ) )
        return;

    QwtStyleSheetRecorder recorder( size() );
    
    QPainter painter( &recorder );
    
    QStyleOption opt;
    opt.initFrom(this);
    style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, this);
    
    painter.end();

    d_data->styleSheet.hasBorder = !recorder.border.rectList.isEmpty();
    d_data->styleSheet.borderPath = recorder.background.path;
    if ( d_data->styleSheet.hasBorder 
        && d_data->styleSheet.borderPath.isEmpty() )
    {
        d_data->styleSheet.borderPath = 
            qwtCombinePathList( rect(), recorder.border.pathList );
    }

    d_data->styleSheet.cornerRects = recorder.clipRects;
    d_data->styleSheet.background.brush = recorder.background.brush;
    d_data->styleSheet.background.origin = recorder.background.origin;
}

QPainterPath QwtPlotCanvas::borderPath( const QRect &rect ) const
{
    if ( testAttribute(Qt::WA_StyledBackground ) )
    {
        QwtStyleSheetRecorder recorder( rect.size() );

        QPainter painter( &recorder );

        QStyleOption opt;
        opt.initFrom(this);
        opt.rect = rect;
        style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, this);

        painter.end();

        if ( !recorder.background.path.isEmpty() )
            return recorder.background.path;

        if ( !recorder.border.rectList.isEmpty() )
            return qwtCombinePathList( rect, recorder.border.pathList );
    }

    return QPainterPath();
}
