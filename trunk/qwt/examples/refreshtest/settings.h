#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <qpen.h>
#include <qbrush.h>

class Settings
{
public:
    enum FunctionType
    {
        NoFunction = -1,

        Wave,
        Noise
    };

    enum UpdateType
    {
        UpdateCanvas,
        RepaintCanvas,
        Replot
    };

    Settings()
    {
        grid.pen = Qt::NoPen;

        curve.brush = Qt::NoBrush;
        curve.numPoints = 1000;
        curve.functionType = Wave;
        curve.paintAttributes = 0;
        curve.renderHint = 0;
        
        canvas.deviceClipping = false;
        canvas.cached = false;
        canvas.paintOnScreen = true;

        updateType = RepaintCanvas;
        updateInterval = 20;
    }

    struct
    {
        QPen pen;
    } grid;

    struct 
    {
        QPen pen;
        QBrush brush;
        uint numPoints;
        FunctionType functionType;
        int paintAttributes;
        int renderHint;
    } curve;

    struct
    {
        bool deviceClipping;
        bool cached;
        bool paintOnScreen;
    } canvas;

    UpdateType updateType;
    int updateInterval;
};

#endif
