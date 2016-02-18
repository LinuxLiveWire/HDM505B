//
// Created by sergey on 05.02.16.
//
#include <QHBoxLayout>
#include <QFrame>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QDebug>

#include "compass.h"


CompassGui::CompassGui( QWidget * parent ):
        QWidget(parent), serialPort(nullptr),
        isClosed(true)
{
    createWidgets();
    createLayouts();
    createConnections();
    bringWidgetsToInitialState();
    bUpdate->click();
}

void CompassGui::createWidgets()
{
    serialControlGroup = new QGroupBox(tr("Channel settings"), this);
    deviceControlGroup = new QGroupBox(tr("Device control"), this);
    devicesGroup = new QGroupBox(tr("Attitude indicator"), this);
    ttys = new QComboBox(this);
    bUpdate = new QPushButton(tr("Update"), this);

    bStart = new QPushButton(tr("Connect"), this);
    bStop = new QPushButton(tr("Disconnect"), this);
    bStop->setEnabled(false);

    bStartCalibr = new QPushButton(tr("Start"), this);
    bStopCalibr = new QPushButton(tr("Stop"), this);
    bStopCalibr->setEnabled(false);
    bSaveCalibr = new QPushButton(tr("Save"), this);
    bSaveCalibr->setEnabled(false);

    bPRH = new QPushButton(tr("PRH"), this);
    bGetDecl = new QPushButton(tr("Read\ndeclination"), this);

    attitude = new AttitudeIndicator(this);
    attitude->setMinimumSize(150, 150);
    attitude->setMaximumSize(200, 200);

    compass = new Compass(this);
    compass->setMinimumSize(150, 150);
    compass->setMaximumSize(200, 200);

    declination = new QDoubleSpinBox(this);
    declination->setRange(-99.9, 99.9);
    declination->setDecimals(1);
    declination->setSingleStep(0.1);
    declination->setValue(0.0);

    bHorizontal = new QRadioButton("Horizontal", this);
    bVertical = new QRadioButton("Vertical", this);

    bAutoReply = new QRadioButton("Auto", this);
    bAnswerReply = new QRadioButton("Manual", this);
}

void CompassGui::createLayouts()
{
    QVBoxLayout * serialControlGroupLayout = new QVBoxLayout;

    QHBoxLayout * serialParamsLayout = new QHBoxLayout;
    serialParamsLayout->addStretch(1);
    serialParamsLayout->addWidget(ttys);
    serialParamsLayout->addWidget(bUpdate);
    serialParamsLayout->addStretch(1);

    serialControlGroup->setLayout(serialParamsLayout);

    QHBoxLayout * serialConnectLayout = new QHBoxLayout;
    serialConnectLayout->addStretch(1);
    serialConnectLayout->addWidget(bStart);
    serialConnectLayout->addWidget(bStop);
    serialConnectLayout->addStretch(2);

    serialControlGroupLayout->addWidget(serialControlGroup);
    serialControlGroupLayout->addLayout(serialConnectLayout);

    QGroupBox   * mountingGroup = new QGroupBox(tr("Mounting mode"), this);
    QHBoxLayout * mountingLayout = new QHBoxLayout;
    mountingLayout->addWidget(bHorizontal);
    mountingLayout->addWidget(bVertical);
    mountingGroup->setLayout(mountingLayout);

    QGroupBox   * angleOutputGroup = new QGroupBox(tr("Output mode"), this);
    QHBoxLayout * angleOutputLayout = new QHBoxLayout;
    angleOutputLayout->addWidget(bAutoReply);
    angleOutputLayout->addWidget(bAnswerReply);
    angleOutputGroup->setLayout(angleOutputLayout);

    QGroupBox   * calibrationGroup = new QGroupBox(tr("Calibration"), this);
    QHBoxLayout * calibrationLayout = new QHBoxLayout;
    calibrationLayout->addWidget(bStartCalibr);
    calibrationLayout->addWidget(bStopCalibr);
    calibrationLayout->addWidget(bSaveCalibr);
    calibrationGroup->setLayout(calibrationLayout);

    QGridLayout * deviceControlLayout = new QGridLayout;
    deviceControlLayout->setRowStretch( 0, 1);
    deviceControlLayout->setRowStretch( 5, 1);
    deviceControlLayout->setColumnStretch( 0, 1);
    deviceControlLayout->setColumnStretch( 4, 1);
    deviceControlLayout->addWidget(calibrationGroup, 1, 1, 1, 3);
    deviceControlLayout->addWidget(angleOutputGroup, 2, 1, 1, 2);
    deviceControlLayout->addWidget(bPRH, 2, 3);
    deviceControlLayout->addWidget(declination, 3, 1 );
    deviceControlLayout->addWidget(bGetDecl, 3, 2);
    deviceControlLayout->addWidget(mountingGroup, 4, 1, 1, 2);

    QFrame  * vLine = new QFrame(this);
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout * indicatorLayout = new QVBoxLayout;
    indicatorLayout->addWidget(attitude);
    indicatorLayout->addWidget(compass);
    devicesGroup->setLayout(indicatorLayout);

    deviceControlGroup->setLayout(deviceControlLayout);

    QHBoxLayout * deviceLayout = new QHBoxLayout;
    deviceLayout->addWidget(deviceControlGroup, 1);
    deviceLayout->addWidget(vLine);
    deviceLayout->addWidget(devicesGroup, 1);

    QFrame  * hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setFrameShadow(QFrame::Sunken);

    QVBoxLayout * widgetLayout = new QVBoxLayout;
    widgetLayout->addLayout(serialControlGroupLayout);
    widgetLayout->addWidget(hLine);
    widgetLayout->addLayout(deviceLayout, 1);
    setLayout(widgetLayout);
    return;
}

void CompassGui::createConnections()
{
    connect( &dataStream, SIGNAL(Error(const QString&)), this, SIGNAL(Error(const QString&)));
    connect( &dataStream, &HDM50Stream::newPacket, [=](int address, QByteArray packet){
        parser.parse(packet);
    });
    connect( &parser, SIGNAL(Error(const QString&)), this, SIGNAL(Error(const QString&)));
    connect( &parser, &HDM50Protocol::PitchRollHeading, [=](qreal pitch, qreal roll, qreal heading){
        //qDebug() << "PRH:" << pitch << roll << heading;
        attitude->setGradient(pitch/90.0);
        attitude->setAngle(-roll);
        compass->setValue(heading);
    });
    connect( &parser, &HDM50Protocol::Declination, [=](qreal angle){
        declination->setValue(angle);
    });
    connect( &parser, &HDM50Protocol::StartCalibration, [=](quint8 code){
        if (code!=255) {
            bStartCalibr->setEnabled(false);
            bStopCalibr->setEnabled(true);
            bSaveCalibr->setEnabled(false);
        } else {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(false);
            bSaveCalibr->setEnabled(false);
            emit Error("Can't start calibration");
        }
    });
    connect( &parser, &HDM50Protocol::StopCalibration, [=](bool ok, quint16 x, quint16 y, quint16 z){
        if (ok) {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(false);
            bSaveCalibr->setEnabled(true);
        } else {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(true);
            bSaveCalibr->setEnabled(false);
            emit Error("Can't stop calibration");
        }
    });
    connect( &parser, &HDM50Protocol::SaveCalibration, [=](bool ok){
        if (ok) {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(false);
            bSaveCalibr->setEnabled(false);
        } else {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(false);
            bSaveCalibr->setEnabled(true);
            emit Error("Can't save calibration");
        }
    });
    connect( &parser, &HDM50Protocol::SaveCalibration, [=](bool ok){
        if (ok) {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(false);
            bSaveCalibr->setEnabled(false);
        } else {
            bStartCalibr->setEnabled(true);
            bStopCalibr->setEnabled(false);
            bSaveCalibr->setEnabled(true);
            emit Error("Can't save calibration");
        }
    });
    connect( &parser, &HDM50Protocol::MountingModeReply, [=](int mode){
        switch (mode) {
            case MM_VERTICAL:
                bVertical->setChecked(true);
                break;
            case MM_HORIZONTAL:
                bHorizontal->setChecked(true);
                break;
            default:
                emit Error("Undefined value of mounting mode");
                break;
        };
    });
    connect( &parser, &HDM50Protocol::OutputModeReply, [=](int mode){
        switch (mode) {
            case AUTO_REPLY:
                bAutoReply->setChecked(true);
                bPRH->setEnabled(false);
                break;
            case ANSWER_REPLY:
                bAnswerReply->setChecked(true);
                bPRH->setEnabled(true);
                break;
            default:
                emit Error("Undefined value of output mode");
                break;
        };
    });
    connect( &parser, &HDM50Protocol::MountingModeSetReply, [=](bool ok){
        if (!ok) {
            bHorizontal->setChecked(bVertical->isChecked());
            emit Error("Can't change mounting mode");
        }
    });
    connect( &parser, &HDM50Protocol::OutputModeSetReply, [=](bool ok){
        if (!ok) {
            bool isAuto = !bAutoReply->isChecked();
            if (isAuto) {
                bPRH->setEnabled(false);
            }
            bAutoReply->setChecked(bAnswerReply->isChecked());
            emit Error("Can't change output mode");
        } else {
            bPRH->setEnabled(bAnswerReply->isChecked());
        }
    });
    connect(bUpdate, SIGNAL(clicked()), this, SLOT(refreshSerials()));
    connect(bStart, SIGNAL(clicked()), this, SLOT(openSerial()));
    connect(bStop, SIGNAL(clicked()), this, SLOT(onStop()));
    connect(bStartCalibr, &QPushButton::clicked, [=](){
        if (serialPort && serialPort->isOpen()) {
            QByteArray cmd = parser.cmd_start_calibrate();
            int res = serialPort->write(cmd);
            if (res!=cmd.size()) {
                if (res<0) {
                    emit Error(QString("Write error: %1").arg(serialPort->errorString()));
                } else {
                    emit Error("Write not complete. Try again.");
                }
            }
        }
    });
    connect(bStopCalibr, &QPushButton::clicked, [=](){
        if (serialPort && serialPort->isOpen()) {
            QByteArray cmd = parser.cmd_stop_calibrate();
            int res = serialPort->write(cmd);
            if (res!=cmd.size()) {
                if (res<0) {
                    emit Error(QString("Write error: %1").arg(serialPort->errorString()));
                } else {
                    emit Error("Write not complete. Try again.");
                }
            }
        }
    });
    connect(bSaveCalibr, &QPushButton::clicked, [=](){
        if (serialPort && serialPort->isOpen()) {
            QByteArray cmd = parser.cmd_save_calibrate();
            int res = serialPort->write(cmd);
            if (res!=cmd.size()) {
                if (res<0) {
                    emit Error(QString("Write error: %1").arg(serialPort->errorString()));
                } else {
                    emit Error("Write not complete. Try again.");
                }
            }
        }
    });
    connect(declination, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value){
        if (serialPort && serialPort->isOpen()) {
            QByteArray cmd = parser.cmd_set_declination(value);
            int res = serialPort->write(cmd);
            if (res!=cmd.size()) {
                if (res<0) {
                    emit Error(QString("Write error: %1").arg(serialPort->errorString()));
                } else {
                    emit Error("Write not complete. Try again.");
                }
            }
        }
    });
    connect(bPRH, &QPushButton::clicked, [=](){
        if (serialPort && serialPort->isOpen()) {
            QByteArray cmd = parser.cmd_read_prh();
            int res = serialPort->write(cmd);
            if (res!=cmd.size()) {
                if (res<0) {
                    emit Error(QString("Write error: %1").arg(serialPort->errorString()));
                } else {
                    emit Error("Write not complete. Try again.");
                }
            }
        }
    });
    connect(bGetDecl, &QPushButton::clicked, [=](){
        if (serialPort && serialPort->isOpen()) {
            QByteArray cmd = parser.cmd_read_declination();
            int res = serialPort->write(cmd);
            if (res!=cmd.size()) {
                if (res<0) {
                    emit Error(QString("Write error: %1").arg(serialPort->errorString()));
                } else {
                    emit Error("Write not complete. Try again.");
                }
            }
        }
    });
    connect(bHorizontal, &QRadioButton::clicked, [=](bool checked){
        if (serialPort && serialPort->isOpen()) {
            if (checked) {
                serialPort->write(parser.cmd_set_mounting_mode(true));
            } else {
                serialPort->write(parser.cmd_set_mounting_mode(false));
            }
        }
    });
    connect(bVertical, &QRadioButton::clicked, [=](bool checked){
        if (serialPort && serialPort->isOpen()) {
            if (checked) {
                serialPort->write(parser.cmd_set_mounting_mode(false));
            } else {
                serialPort->write(parser.cmd_set_mounting_mode(true));
            }
        }
    });
    connect(bAutoReply, &QRadioButton::clicked, [=](bool checked){
        if (serialPort && serialPort->isOpen()) {
            if (checked) {
                serialPort->write(parser.cmd_set_angle_output_mode(true));
            } else {
                serialPort->write(parser.cmd_set_angle_output_mode(false));
            }
        }
    });
    connect(bAnswerReply, &QRadioButton::clicked, [=](bool checked){
        if (serialPort && serialPort->isOpen()) {
            if (checked) {
                serialPort->write(parser.cmd_set_angle_output_mode(false));
            } else {
                serialPort->write(parser.cmd_set_angle_output_mode(true));
            }
        }
    });
}

void CompassGui::bringWidgetsToInitialState()
{
    bStop->setEnabled(false);
    deviceControlGroup->setEnabled(false);
    devicesGroup->setEnabled(false);
}

void CompassGui::refreshSerials()
{
    int ind = -1;
    QString curr = ttys->currentText();
    ttys->clear();
    QList<QSerialPortInfo> serials = QSerialPortInfo::availablePorts();
    foreach(const QSerialPortInfo& port, serials){
        //qDebug() << port.description() << port.manufacturer() << port.systemLocation();
        ttys->addItem(port.portName(), QVariant(port.systemLocation()));
    }
    if (curr.size() && (ind = ttys->findText(curr))!=-1) {
        ttys->setCurrentIndex(ind);
    }
    return;
}

void CompassGui::openSerial()
{
    if (serialPort && serialPort->isOpen()) {
        return;
    }
    QString curr = ttys->currentText();
    if (!curr.length()) {
        return;
    }
    serialPort = new QSerialPort(curr);
    connect(serialPort, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            [=](QSerialPort::SerialPortError error){
                if (serialPort) {
                    if (error != QSerialPort::NoError) {
                        emit Error(serialPort->errorString());
                    }
                }
                return;
            });
    bool isOpen = serialPort->open(QIODevice::ReadWrite);
    if (!isOpen) {
        emit Error(serialPort->errorString());
        delete serialPort;
        serialPort = nullptr;
        return;
    }
    isClosed = false;
    dataStream.reset();
    connect(serialPort, SIGNAL(aboutToClose()), this, SLOT(closeSerial()));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    serialControlGroup->setEnabled(false);
    deviceControlGroup->setEnabled(true);
    devicesGroup->setEnabled(true);
    bStart->setEnabled(false);
    bStop->setEnabled(true);
    QTimer::singleShot( 500, [=](){
        serialPort->write(parser.cmd_get_output_mode());
    } );
    QTimer::singleShot( 750, [=](){
        serialPort->write(parser.cmd_get_mounting_mode());
    } );
    QTimer::singleShot( 1000, [=](){
        serialPort->write(parser.cmd_read_declination());
    } );
}

void CompassGui::onStop() {
    if (serialPort) {
        serialPort->close();
        delete serialPort;
        serialPort = nullptr;
    }
}

void CompassGui::closeSerial()
{
    if (isClosed) {
        return;
    }
    isClosed = true;
    serialControlGroup->setEnabled(true);
    deviceControlGroup->setEnabled(false);
    devicesGroup->setEnabled(false);
    bStart->setEnabled(true);
    bStop->setEnabled(false);
}

void CompassGui::readData()
{
    char byte = 0;
    qint64 len;
    while ((len = serialPort->read(&byte, 1))>0) {
        dataStream.put(byte);
    }
    if (len<0) {
        emit Error(serialPort->errorString());
    }
}

AppWindow::AppWindow(QWidget *parent)
        : QMainWindow(parent)
{
    centralWidget = new CompassGui( parent );
    setCentralWidget( centralWidget );
    connect(centralWidget, SIGNAL(Error(const QString&)), this, SLOT(onError(const QString&)));
}

void AppWindow::onError(const QString& error)
{
    QStatusBar  * sb = statusBar();
    sb->showMessage(error, 10000);
}

AppWindow::~AppWindow()
{
    delete centralWidget;
}
