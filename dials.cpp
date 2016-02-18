//
// Created by sergey on 10.02.16.
//

#include "dials.h"

#include <qwt_point_polar.h>
#include <qwt_round_scale_draw.h>
#include <qevent.h>
#include <qwt_round_scale_draw.h>

#include <QPainter>
#include <QPalette>

QPalette colorTheme( const QColor &base )
{
    QPalette palette;
    palette.setColor( QPalette::Base, base );
    palette.setColor( QPalette::Window, base.dark( 150 ) );
    palette.setColor( QPalette::Mid, base.dark( 110 ) );
    palette.setColor( QPalette::Light, base.light( 170 ) );
    palette.setColor( QPalette::Dark, base.dark( 170 ) );
    palette.setColor( QPalette::Text, base.dark( 200 ).light( 800 ) );
    palette.setColor( QPalette::WindowText, base.dark( 200 ) );

    return palette;
}

AttitudeIndicatorNeedle::AttitudeIndicatorNeedle( const QColor &color )
{
    QPalette palette;
    palette.setColor( QPalette::Text, color );
    setPalette( palette );
}

void AttitudeIndicatorNeedle::drawNeedle( QPainter *painter,
                                          double length, QPalette::ColorGroup colorGroup ) const
{
    double triangleSize = length * 0.1;
    double pos = length - 2.0;

    QPainterPath path;
    path.moveTo( pos, 0 );
    path.lineTo( pos - 2 * triangleSize, triangleSize );
    path.lineTo( pos - 2 * triangleSize, -triangleSize );
    path.closeSubpath();

    painter->setBrush( palette().brush( colorGroup, QPalette::Text ) );
    painter->drawPath( path );

    double l = length - 2;
    painter->setPen( QPen( palette().color( colorGroup, QPalette::Text ), 3 ) );
    painter->drawLine( QPointF( 0.0, -l ), QPointF( 0.0, l ) );
}

AttitudeIndicator::AttitudeIndicator( QWidget *parent ):
        QwtDial( parent ), d_gradient( 0.0 )
{
    QwtRoundScaleDraw *scaleDraw = new QwtRoundScaleDraw();
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Labels, false );
    scaleDraw->setPenWidth(3);
    setScaleDraw( scaleDraw );

    setMode( RotateScale );
    setWrapping( true );

    setOrigin( 270.0 );

    setScaleMaxMinor( 0 );
    setScaleStepSize( 45.0 );
    setScale( 0.0, 360.0 );

    setReadOnly(true);
    setLineWidth(4);
    setFrameShadow(QwtDial::Sunken);

    const QColor color = palette().color( QPalette::Text );
    setNeedle( new AttitudeIndicatorNeedle( color ) );
    setPalette( colorTheme( QColor( Qt::lightGray ).dark( 150 ) ) );
}

void AttitudeIndicator::setGradient( double gradient )
{
    if ( gradient < -1.0 )
        gradient = -1.0;
    else if ( gradient > 1.0 )
        gradient = 1.0;

    if ( d_gradient != gradient )
    {
        d_gradient = gradient;
        update();
    }
}

void AttitudeIndicator::drawScale( QPainter *painter,
                                   const QPointF &center, double radius ) const
{
    const double offset = 4.0;

    const QPointF p0 = qwtPolar2Pos( center, offset, 1.5 * M_PI );

    const double w = innerRect().width();

    QPainterPath path;
    path.moveTo( qwtPolar2Pos( p0, w, 0.0 ) );
    path.lineTo( qwtPolar2Pos( path.currentPosition(), 2 * w, M_PI ) );
    path.lineTo( qwtPolar2Pos( path.currentPosition(), w, 0.5 * M_PI ) );
    path.lineTo( qwtPolar2Pos( path.currentPosition(), w, 0.0 ) );

    painter->save();
    painter->setClipPath( path ); // swallow 180 - 360 degrees

    QwtDial::drawScale( painter, center, radius );

    painter->restore();
}

void AttitudeIndicator::drawScaleContents( QPainter *painter,
                                           const QPointF &, double ) const
{
    int dir = 360 - qRound( origin() - value() ); // counter clockwise
    int arc = 90 + qRound( gradient() * 90 );

    const QColor skyColor( 38, 151, 221 );

    painter->save();
    painter->setBrush( skyColor );
    painter->drawChord( scaleInnerRect(),
                        ( dir - arc ) * 16, 2 * arc * 16 );
    painter->restore();
}

void AttitudeIndicator::setAngle(double angle)
{
    if ( angle < 0.0 ) {
        angle = 360.0 + angle;
    }
    setValue( angle );
}

Compass::Compass(QWidget * parent):
        QwtCompass(parent)
{
    setLineWidth( 4 );
    setFrameShadow(QwtCompass::Sunken);
    QwtCompassScaleDraw *scaleDraw = new QwtCompassScaleDraw();
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Ticks, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Labels, true );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 3 );

    setScaleDraw( scaleDraw );

    setNeedle( new QwtCompassMagnetNeedle( QwtCompassMagnetNeedle::TriangleStyle, Qt::gray, Qt::red ) );
    QPalette palette0;
    int c;
    for ( c = 0; c < QPalette::NColorRoles; c++ )
    {
        const QPalette::ColorRole colorRole =
                static_cast<QPalette::ColorRole>( c );

        palette0.setColor( colorRole, QColor() );
    }

    palette0.setColor( QPalette::Base,
                       palette().color( backgroundRole() ).light( 120 ) );
    palette0.setColor( QPalette::WindowText,
                       palette0.color( QPalette::Base ) );
    QPalette newPalette = palette();
    for ( c = 0; c < QPalette::NColorRoles; c++ )
    {
        const QPalette::ColorRole colorRole =
                static_cast<QPalette::ColorRole>( c );

        if ( palette0.color( colorRole ).isValid() )
            newPalette.setColor( colorRole, palette0.color( colorRole ) );
    }

    for ( int i = 0; i < QPalette::NColorGroups; i++ )
    {
        const QPalette::ColorGroup colorGroup =
                static_cast<QPalette::ColorGroup>( i );

        const QColor light =
                newPalette.color( colorGroup, QPalette::Base ).light( 170 );
        const QColor dark = newPalette.color( colorGroup, QPalette::Base ).dark( 170 );
        const QColor mid = frameShadow() == QwtDial::Raised
                           ? newPalette.color( colorGroup, QPalette::Base ).dark( 110 )
                           : newPalette.color( colorGroup, QPalette::Base ).light( 110 );

        newPalette.setColor( colorGroup, QPalette::Dark, dark );
        newPalette.setColor( colorGroup, QPalette::Mid, mid );
        newPalette.setColor( colorGroup, QPalette::Light, light );
    }

    setPalette( newPalette );
}
