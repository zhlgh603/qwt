#include <qwidget.h>

class Plot;
class Knob;
class WheelBox;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget * = NULL);

    double amplitude() const;
    double frequency() const;
    double signalInterval() const;

signals:
    void amplitudeChanged(double);
    void frequencyChanged(double);
    void signalIntervalChanged(double);

private:
    Knob *d_frequencyKnob;
    Knob *d_amplitudeKnob;
    WheelBox *d_timerWheel;
    WheelBox *d_intervalWheel;

    Plot *d_plot;
};
