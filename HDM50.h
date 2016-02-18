//
// Created by sergey on 08.02.16.
//
#include <QObject>
#include <QByteArray>

#ifndef HDM505B_HDM50_H
#define HDM505B_HDM50_H

typedef enum {FINISHED, START, ACCUMULATE} ParseStatus;
typedef enum {MM_HORIZONTAL, MM_VERTICAL} MountingMode;
typedef enum {AUTO_REPLY, ANSWER_REPLY} OutputMode;

class HDM50Protocol: public QObject {
    Q_OBJECT
public:
    HDM50Protocol(QObject* parent=0);
    void parse_prh(const QByteArray&);
    void parse_declination_reply(const QByteArray&);
    void parse_read_declination(const QByteArray&);
    void parse_start_calibration(const QByteArray&);
    void parse_stop_calibration(const QByteArray&);
    void parse_save_calibration(const QByteArray&);
    void parse_answer_mounting_mode(const QByteArray&);
    void parse_answer_output_mode(const QByteArray&);
    void parse_output_mode_reply(const QByteArray&);
    void parse_mounting_mode_reply(const QByteArray&);
signals:
    void PitchRollHeading(qreal, qreal, qreal);
    void DeclinationAnswer(bool);
    void Declination(qreal);
    void StartCalibration(quint8);
    void StopCalibration(bool, quint16, quint16, quint16);
    void SaveCalibration(bool);
    void OutputModeReply(int);
    void MountingModeReply(int);
    void OutputModeSetReply(bool);
    void MountingModeSetReply(bool);
    void Error(const QString&);
public slots:
    void parse(const QByteArray&);
public:
    QByteArray cmd_read_prh() const {
        return QByteArray(_get_prh, sizeof(_get_prh));
    }
    QByteArray cmd_start_calibrate() const {
        return QByteArray(_start_calibrate, sizeof(_start_calibrate));
    }
    QByteArray cmd_stop_calibrate() const {
        return QByteArray(_stop_calibrate, sizeof(_stop_calibrate));
    }
    QByteArray cmd_save_calibrate() const {
        return QByteArray(_save_calibrate, sizeof(_save_calibrate));
    }
    QByteArray cmd_get_mounting_mode() const {
        return QByteArray(_get_mounting_mode, sizeof(_get_mounting_mode));
    }
    QByteArray cmd_get_output_mode() const {
        return QByteArray(_get_output_mode, sizeof(_get_output_mode));
    }
    QByteArray cmd_read_declination() const {
        return QByteArray(_get_declination, sizeof(_get_declination));
    }
    QByteArray cmd_set_declination(qreal) const;
    QByteArray cmd_set_baud_rate(quint32) const;
    QByteArray cmd_set_module_address(quint8) const;
    QByteArray cmd_set_angle_output_mode(bool auto_mode=false) const;
    QByteArray cmd_set_mounting_mode(bool horizontal=true) const;
private:
    const char _get_prh[5] = {0x68, 0x04, 0x00, 0x04, 0x08};
    const char _start_calibrate[5] = {0x68, 0x04, 0x00, 0x08, 0x0C};
    const char _stop_calibrate[5] = {0x68, 0x04, 0x00, 0x09, 0x0D};
    const char _save_calibrate[5] = {0x68, 0x04, 0x00, 0x0A, 0x0E};
    const char _set_declination[7] = {0x68, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00};
    const char _get_declination[5] = {0x68, 0x04, 0x00, 0x07, 0x0b};
    const char _set_baud_rate[6] = {0x68, 0x05, 0x00, 0x0B, 0x00, 0x00};
    const char _set_module_address[6] = {0x68, 0x05, 0x00, 0x0F, 0x00, 0x00};
    const char _set_output_mode[6] = {0x68, 0x05, 0x00, 0x0C, 0x00, 0x00};
    const char _set_mounting_mode[6] = {0x68, 0x05, 0x00, 0x2A, 0x00, 0x00};
    const char _get_mounting_mode[5] = {0x68, 0x04, 0x00, 0x41, 0x45};
    const char _get_output_mode[5] = {0x68, 0x04, 0x00, 0x42, 0x46};
};

class HDM50Stream: public QObject {
    Q_OBJECT
public:
    HDM50Stream(QObject* parent=0);
    QByteArray get() const;
signals:
    void newPacket(int, QByteArray);
    void Error(const QString&);
public slots:
    void put(unsigned char);
    void reset();
private:
    ParseStatus status;
    QByteArray data;
    int length;
    int lost;
    int address;
    quint64 rx;
};

#endif //HDM505B_HDM50_H
