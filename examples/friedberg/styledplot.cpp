#include "styledplot.h"
#include <qwt_plot_canvas.h>
#include <qwt_text_label.h>
#include <qwt_legend.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qevent.h>

StyledPlot::StyledPlot( QWidget *parent ):
    QwtPlot( parent )
{
    canvas()->installEventFilter( this );
    initStyleSheets();
}

void StyledPlot::drawCanvas( QPainter *painter )
{
    if ( !d_canvasClip.isEmpty() )
        painter->setClipRegion( d_canvasClip, Qt::IntersectClip );

    QwtPlot::drawCanvas( painter );
}

void StyledPlot::updateCanvasClip()
{
    // The rounded borders of the styled background need to
    // clipped away, to avoid that we paint on them.

    if ( !canvas()->testAttribute( Qt::WA_StyledBackground ) )
    {
        d_canvasClip = QRegion();
        return;
    }

    // Let the style paint the styled background
    // to an image

    QImage image( size(), QImage::Format_ARGB32 );
    image.fill( Qt::transparent );

    QPainter painter( &image );

    QStyleOption opt;
    opt.initFrom( canvas() );
    style()->drawPrimitive( QStyle::PE_Widget, &opt, &painter, canvas() );
    painter.end();

    // Assuming we have no border in the center and solid background
    // color inside the frame we can calculate the mask from the
    // pixel in the center.

    const QRgb bg = image.pixel( image.width() / 2, image.height() / 2 );

    QImage mask = image.createMaskFromColor( bg, Qt::MaskOutColor );
    d_canvasClip = QBitmap::fromImage( mask );
}

void StyledPlot::initStyleSheets()
{
    const QColor baseColor( 231, 239, 247 );
    const QColor borderColor( 133, 190, 232 );
    const QColor canvasColor( Qt::lightGray );

    QString styleSheet;

    styleSheet = QString(
        "border: 1px solid white;"
        "border-radius: 10px;"
        "padding: 0px;"
        "background-color: qlineargradient( x1: 0, y1: 0, x2: 0, y2: 1, "
        "stop: 0 %1, stop: 0.5 white, stop: 1 %2 );"
    );

    styleSheet = styleSheet.arg( baseColor.name() ).arg( baseColor.name() );

    setStyleSheet( styleSheet );

    styleSheet = QString(
        "border: 3px solid %1;"
        "border-radius: 10px;"
        "background-color: %2;"
    );

    styleSheet = styleSheet.arg( borderColor.name() ).arg( canvasColor.name() );

    canvas()->setStyleSheet( styleSheet );

    styleSheet = QString(
        "border: 0px;"
        "background-color: transparent;"
    );

    if ( legend() )
        legend()->setStyleSheet( styleSheet );

    titleLabel()->setStyleSheet( styleSheet );
}

bool StyledPlot::event( QEvent * event )
{
    switch( event->type() )
    {
        case QEvent::ChildPolished:
        {
            QChildEvent *childEvent = static_cast<QChildEvent *>( event );
            if ( qobject_cast<const QwtLegend *>( childEvent->child() ) )
                initStyleSheets(); 

            break;
        }
        default:
            break;
    }

    return QwtPlot::event( event );
}

bool StyledPlot::eventFilter( QObject* object, QEvent* event )
{
    if ( object == canvas() )
    {
        if ( event->type() == QEvent::Resize ||
                event->type() == QEvent::StyleChange )
        {
            // Calculating the canvas clip is an expensive
            // operation. So we better do this in advance and not
            // for every paint event.

            updateCanvasClip();
        }
    }

    return QwtPlot::eventFilter( object, event );
}

