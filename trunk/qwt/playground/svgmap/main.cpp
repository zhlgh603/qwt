/*****************************************************************************
 * Qwt Examples - Copyright (C) 2002 Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#include "plot.h"

#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

class MainWindow: public QMainWindow
{
public:
    MainWindow( const QString &fileName )
    {
        Plot *plot = new Plot( this );
        if ( !fileName.isEmpty() )
            plot->loadSVG( fileName );

        setCentralWidget( plot );

#ifndef QT_NO_FILEDIALOG

        QToolBar *toolBar = new QToolBar( this );

        QToolButton *btnLoad = new QToolButton( toolBar );

        btnLoad->setText( "Load SVG" );
        btnLoad->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
        toolBar->addWidget( btnLoad );

        addToolBar( toolBar );

        connect( btnLoad, SIGNAL( clicked() ), plot, SLOT( loadSVG() ) );
#endif
    }
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QString fileName;
    if ( argc > 1 )
        fileName = argv[1];

    MainWindow w( fileName );
    w.resize( 600, 400 );
    w.show();

    int rv = a.exec();
    return rv;
}
