#include "editor.h"
#include <qwt_plot.h>
#include <qwt_scale_map.h>
#include <qwt_plot_shapeitem.h>
#include <qevent.h>

class Overlay: public QwtPlotOverlay
{
public:
    Overlay( QWidget *parent, Editor *editor ):
        QwtPlotOverlay( parent ),
        d_editor( editor )
    {
    }

protected:
    virtual void drawOverlay( QPainter *painter ) const
    {
        d_editor->drawOverlay( painter );
    }

    virtual QRegion maskHint() const
    {
        return d_editor->maskHint();
    }

private:
    Editor *d_editor;
};

Editor::Editor( QwtPlot* plot ):
    QObject( plot ),
    m_isEnabled( false ),
    m_overlay( NULL )
{
    setEnabled( true );
}


Editor::~Editor()
{
    delete m_overlay;
}

QwtPlot *Editor::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *Editor::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}


void Editor::setEnabled( bool on )
{
    if ( on == m_isEnabled )
        return;

    QwtPlot *plot = qobject_cast<QwtPlot *>( parent() );
    if ( plot )
    {
        m_isEnabled = on;

        if ( on )
        {
            plot->canvas()->installEventFilter( this );
        }
        else
        {
            plot->canvas()->removeEventFilter( this );

            delete m_overlay;
            m_overlay = NULL;
        }
    }
}

bool Editor::isEnabled() const
{
    return m_isEnabled;
}

bool Editor::eventFilter( QObject* object, QEvent* event )
{
    QwtPlot *plot = qobject_cast<QwtPlot *>( parent() );
    if ( plot && object == plot->canvas() )
    {
        switch( event->type() )
        {
            case QEvent::MouseButtonPress:
            {
                const QMouseEvent* mouseEvent =
                    dynamic_cast<QMouseEvent* >( event );

                if ( m_overlay == NULL && 
                    mouseEvent->button() == Qt::LeftButton  )
                {
                    const bool accepted = pressed( mouseEvent->pos() );
                    if ( accepted )
                    {
                        m_overlay = new Overlay( plot->canvas(), this );
                        m_overlay->updateOverlay();
                        m_overlay->show();
                    }
                }

                break;
            }
            case QEvent::MouseMove:
            {
                if ( m_overlay )
                {
                    const QMouseEvent* mouseEvent =
                        dynamic_cast< QMouseEvent* >( event );

                    const bool accepted = moved( mouseEvent->pos() );
                    if ( accepted )
                        m_overlay->updateOverlay();
                }

                break;
            }
            case QEvent::MouseButtonRelease:
            {
                const QMouseEvent* mouseEvent =
                    static_cast<QMouseEvent* >( event );

                if ( m_overlay && mouseEvent->button() == Qt::LeftButton )
                {
                    released( mouseEvent->pos() );

                    delete m_overlay;
                    m_overlay = NULL;
                }

                break;
            }
            default:
                break;
        }

        return false;
    }

    return QObject::eventFilter( object, event );
}

bool Editor::pressed( const QPoint& pos )
{
    m_editedItem = itemAt( pos );
    if ( m_editedItem )
    {
        m_currentPos = pos;
        m_editedItem->setVisible( false );

        if ( plot() )
            plot()->replot();

        return true;
    }

    return false; // don't accept the position
}

bool Editor::moved( const QPoint& pos )
{
    if ( plot() == NULL )
        return false;

    const QwtScaleMap xMap = plot()->canvasMap( m_editedItem->xAxis() );
    const QwtScaleMap yMap = plot()->canvasMap( m_editedItem->yAxis() );

    const QPointF p1 = QwtScaleMap::invTransform( xMap, yMap, m_currentPos );
    const QPointF p2 = QwtScaleMap::invTransform( xMap, yMap, pos );

    m_editedItem->setShape( m_editedItem->shape().translated( p2 - p1 ) );
    m_currentPos = pos;

    return true;
}

void Editor::released( const QPoint& pos )
{
    Q_UNUSED( pos );

    if ( m_editedItem  )
    {
        raiseItem( m_editedItem );
        m_editedItem->setVisible( true );
        m_editedItem = NULL;

        if ( plot() )
            plot()->replot();
    }
}

QwtPlotShapeItem* Editor::itemAt( const QPoint& pos ) const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL )
        return NULL;

    // translate pos into the plot coordinates
    double coords[ QwtPlot::axisCnt ];
    coords[ QwtPlot::xBottom ] =
        plot->canvasMap( QwtPlot::xBottom ).invTransform( pos.x() );
    coords[ QwtPlot::xTop ] =
        plot->canvasMap( QwtPlot::xTop ).invTransform( pos.x() );
    coords[ QwtPlot::yLeft ] =
        plot->canvasMap( QwtPlot::yLeft ).invTransform( pos.y() );
    coords[ QwtPlot::yRight ] =
        plot->canvasMap( QwtPlot::yRight ).invTransform( pos.y() );

    QwtPlotItemList items = plot->itemList();
    for ( int i = items.size() - 1; i >= 0; i-- )
    {
        QwtPlotItem *item = items[ i ];
        if ( item->isVisible() &&
            item->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            QwtPlotShapeItem *shapeItem = static_cast<QwtPlotShapeItem *>( item );
            const QPointF p( coords[ item->xAxis() ], coords[ item->yAxis() ] );

            if ( shapeItem->boundingRect().contains( p )
                && shapeItem->shape().contains( p ) )
            {
                return shapeItem;
            }
        }
    }

    return NULL;
}

QRegion Editor::maskHint() const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL || m_editedItem == NULL )
        return QRegion();

    const QwtScaleMap xMap = plot->canvasMap( m_editedItem->xAxis() );
    const QwtScaleMap yMap = plot->canvasMap( m_editedItem->yAxis() );

    QRect rect = QwtScaleMap::transform( xMap, yMap,
        m_editedItem->shape().boundingRect() ).toRect();

    const int m = 5; // for the pen
    return rect.adjusted( -m, -m, m, m );
}

void Editor::drawOverlay( QPainter* painter ) const
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL || m_editedItem == NULL )
        return;

    const QwtScaleMap xMap = plot->canvasMap( m_editedItem->xAxis() );
    const QwtScaleMap yMap = plot->canvasMap( m_editedItem->yAxis() );

    painter->setRenderHint( QPainter::Antialiasing,
        m_editedItem->testRenderHint( QwtPlotItem::RenderAntialiased ) );
    m_editedItem->draw( painter, xMap, yMap,
        plot->canvas()->contentsRect() );
}

void Editor::raiseItem( QwtPlotShapeItem *shapeItem )
{
    const QwtPlot *plot = this->plot();
    if ( plot == NULL || shapeItem == NULL )
        return;

    const QwtPlotItemList items = plot->itemList();
    for ( int i = items.size() - 1; i >= 0; i-- )
    {
        QwtPlotItem *item = items[ i ];
        if ( shapeItem == item )
            return;

        if ( item->isVisible() &&
            item->rtti() == QwtPlotItem::Rtti_PlotShape )
        {
            shapeItem->setZ( item->z() + 1 );
            return;
        }
    }
}

