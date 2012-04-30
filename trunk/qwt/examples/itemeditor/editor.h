#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <qobject.h>
#include <qregion.h>
#include <qpointer.h>
#include <qwt_plot_overlay.h>

class QwtPlot;
class QwtPlotShapeItem;
class QPainter;
class QPoint;

class Editor: public QObject
{
    Q_OBJECT

public:
    enum Mode
    {
        NoMask,
        Mask,
        AlphaMask,
        AlphaMaskRedraw,
        AlphaMaskCopyMask
    };

    Editor( QwtPlot * );
    virtual ~Editor();

    const QwtPlot *plot() const;
    QwtPlot *plot();

    virtual void setEnabled( bool on );
    bool isEnabled() const;

    void drawOverlay( QPainter * ) const;
    QRegion maskHint() const;

    virtual bool eventFilter( QObject *, QEvent *);

    void setMode( Mode mode );
    Mode mode() const;

private:
    bool pressed( const QPoint & );
    bool moved( const QPoint & );
    void released( const QPoint & );

    QwtPlotShapeItem* itemAt( const QPoint& ) const;
    void raiseItem( QwtPlotShapeItem * );

    bool d_isEnabled;
    QPointer<QwtPlotOverlay> d_overlay;

    // Mouse positions
    QPointF d_currentPos;
    QwtPlotShapeItem* d_editedItem;

    Mode d_mode;
};

#endif
