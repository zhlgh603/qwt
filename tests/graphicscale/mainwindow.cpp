#include "mainwindow.h"
#include "canvas.h"
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qfiledialog.h>

MainWindow::MainWindow()
{
    QWidget *w = new QWidget( this );

    d_canvas[0] = new Canvas( Canvas::Native, this );
    d_canvas[0]->setAutoFillBackground( true );
    d_canvas[0]->setPalette( Qt::gray );

    d_canvas[1] = new Canvas( Canvas::VectorGraphic, this );
    d_canvas[1]->setAutoFillBackground( true );
    d_canvas[1]->setPalette( Qt::gray );

    QVBoxLayout *vBox1 = new QVBoxLayout();
    vBox1->setContentsMargins( 0, 0, 0, 0 );
    vBox1->setSpacing( 5 );
    vBox1->addWidget( new QLabel( "Native" ), 0, Qt::AlignCenter );
    vBox1->addWidget( d_canvas[0], 10 );

    QVBoxLayout *vBox2 = new QVBoxLayout();
    vBox2->setContentsMargins( 0, 0, 0, 0 );
    vBox2->setSpacing( 5 );
    vBox2->addWidget( new QLabel( "Vector Graphic" ), 0, Qt::AlignCenter );
    vBox2->addWidget( d_canvas[1], 10 );

    QHBoxLayout *layout = new QHBoxLayout( w );
    layout->addLayout( vBox1 );
    layout->addLayout( vBox2 );

    setCentralWidget( w );

    QToolBar *toolBar = new QToolBar( this );

    QToolButton *btnLoad = new QToolButton( toolBar );

    btnLoad->setText( "Load SVG" );
    btnLoad->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( btnLoad );

    addToolBar( toolBar );

    connect( btnLoad, SIGNAL( clicked() ), this, SLOT( loadSVG() ) );
};

MainWindow::~MainWindow()
{
}

void MainWindow::loadSVG()
{
    QString dir = "svg";
    const QString fileName = QFileDialog::getOpenFileName( NULL,
        "Load a Scaleable Vector Graphic (SVG) Document",
        dir, "SVG Files (*.svg)" );

    if ( !fileName.isEmpty() )
        loadSVG( fileName );
}

void MainWindow::loadSVG( const QString &fileName )
{
    QFile file( fileName );
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        return;

    const QByteArray document = file.readAll();
    file.close();

    d_canvas[0]->setSvg( document );
    d_canvas[1]->setSvg( document );
}

