#ifndef COMPASS_H
#define COMPASS_H

#include <QMainWindow>
#include <QWidget>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QtSerialPort>
#include <QDoubleSpinBox>
#include <QRadioButton>

#include "dials.h"
#include "HDM50.h"

class CompassGui: public QWidget {
Q_OBJECT
public:
    CompassGui( QWidget* parent=nullptr );
protected slots:
    void refreshSerials();
    void readData();
    void openSerial();
    void closeSerial();
    void onStop();
signals:
    void Error(const QString&);
private:
    void createWidgets();
    void createLayouts();
    void createConnections();
    void bringWidgetsToInitialState();
private:
    QGroupBox   * serialControlGroup;
    QGroupBox   * deviceControlGroup;
    QGroupBox   * devicesGroup;
    QComboBox   * ttys;
    QPushButton * bStart, * bStop, * bUpdate;
    QPushButton * bStartCalibr, * bStopCalibr, * bSaveCalibr;
    QPushButton * bPRH, * bGetDecl;
    QSerialPort * serialPort;
    AttitudeIndicator   * attitude;
    Compass     * compass;
    QDoubleSpinBox      * declination;
    QRadioButton    * bHorizontal, * bVertical;
    QRadioButton    * bAutoReply, * bAnswerReply;
    bool isClosed; // signal aboutToClose called many times per one close
    HDM50Stream dataStream;
    HDM50Protocol parser;
};

class AppWindow : public QMainWindow
{
Q_OBJECT

public:
    AppWindow(QWidget * parent = 0);
    ~AppWindow();
public slots:
    void onError(const QString&);
private:
    CompassGui  * centralWidget;
};

#endif // COMPASS_H
