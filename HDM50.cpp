//
// Created by sergey on 08.02.16.
//

#include "HDM50.h"
#include <QDebug>

// 2400, 4800, 9600, 19200, 38400, 115200 - baud rates

qint16 _1byte2int(const QByteArray& data)
{
    quint16 x10 = ((quint8)((data[0]&0xF0)>>4))*10;
    quint16 x1 = ((quint8)((data[0]&0x0F)>>4));
    return x10+x1;
}

qreal _3bytes2real(const QByteArray& data)
{
    qreal x100 = ((quint8)data[0]&0x0F)*100.0;
    qreal x10 = ((quint8)((data[1]&0xF0)>>4))*10.0;
    qreal x1 = ((quint8)data[1]&0x0F)*1.0;
    qreal d10 = ((quint8)((data[2]&0xF0)>>4))/10.0;
    qreal d100 = ((quint8)data[2]&0x0F)/100.0;
    qreal sign = ((quint8)data[0]&0xF0)?-1.0:1.0;
    return sign*(x100+x10+x1+d10+d100);
}

qreal _2bytes2real(const QByteArray& data)
{
    qreal x10 = ((quint8)data[0]&0x0F)*10.0;
    qreal x1 = ((quint8)((data[1]&0xF0)>>4))*1.0;
    qreal d10 = ((quint8)data[1]&0x0f)/10.0;
    qreal sign = ((quint8)data[0]&0xF0)?-1.0:1.0;
    return sign*(x10+x1+d10);
}

qreal bytes2real(const QByteArray& data)
{
    switch (data.size()) {
        case 3:
            return _3bytes2real(data);
        case 2:
            return _2bytes2real(data);
        default:
            return 0.0;
    };
}

HDM50Protocol::HDM50Protocol(QObject* parent):
    QObject(parent)
{

}

void HDM50Protocol::parse(const QByteArray& packet)
{
    if (packet.size()==0) {
        emit Error("Zero length packet");
        return;
    }
    unsigned char responce = packet[0];
    //qDebug() << "Reply:" << QString::number( responce, 16);
    switch (responce) {
        case 0x84:
            parse_prh(packet.mid(1));
            break;
        case 0x86:
            parse_declination_reply(packet.mid(1));
            break;
        case 0x87:
            parse_read_declination(packet.mid(1));
            break;
        case 0x88:
            parse_start_calibration(packet.mid(1));
            break;
        case 0x89:
            parse_stop_calibration(packet.mid(1));
            break;
        case 0x8A:
            parse_save_calibration(packet.mid(1));
            break;
        case 0x8B:
            break;
        case 0x8F:
            break;
        case 0x8C:
            parse_output_mode_reply(packet.mid(1));
            break;
        case 0xAA:
            parse_mounting_mode_reply(packet.mid(1));
            break;
        case 0xC1:
            parse_answer_mounting_mode(packet.mid(1));
            break;
        case 0xC2:
            parse_answer_output_mode(packet.mid(1));
            break;
        default:
            emit Error( QString("Undefined command descriptor: 0x%1").arg(QString::number((uint)responce, 16)) );
            break;
    };
}

void HDM50Protocol::parse_prh(const QByteArray& data)
{
    Q_ASSERT(data.size()==9);
    qreal pitch = bytes2real(data.mid(0, 3));
    qreal roll = bytes2real(data.mid(3, 3));
    qreal heading = bytes2real(data.mid(6, 3));
    emit PitchRollHeading(pitch, roll, heading);
}

void HDM50Protocol::parse_declination_reply(const QByteArray& data)
{
    Q_ASSERT(data.size()==1);
    bool ok = (data[0]==0)?true:false;
    emit DeclinationAnswer(ok);
}

void HDM50Protocol::parse_read_declination(const QByteArray& data)
{
    Q_ASSERT(data.size()==2);
    qreal declination = bytes2real(data);
    emit Declination(declination);
}

void HDM50Protocol::parse_start_calibration(const QByteArray & data)
{
    Q_ASSERT(data.size()==1);
    emit StartCalibration((quint8)data[0]);
}

void HDM50Protocol::parse_answer_mounting_mode(const QByteArray& data)
{
    Q_ASSERT(data.size()==1);
    int mode = (data[0]==0)?MM_HORIZONTAL:MM_VERTICAL;
    emit MountingModeReply(mode);
}

void HDM50Protocol::parse_answer_output_mode(const QByteArray& data)
{
    Q_ASSERT(data.size()==1);
    int mode = (data[0]==0)?ANSWER_REPLY:AUTO_REPLY;
    emit OutputModeReply(mode);
}

void HDM50Protocol::parse_output_mode_reply(const QByteArray& data)
{
    Q_ASSERT(data.size()==1);
    bool ok = (data[0]==0)?true:false;
    emit OutputModeSetReply(ok);
}

void HDM50Protocol::parse_mounting_mode_reply(const QByteArray& data)
{
    Q_ASSERT(data.size()==1);
    bool ok = (data[0]==0)?true:false;
    emit MountingModeSetReply(ok);
}

void HDM50Protocol::parse_stop_calibration(const QByteArray& data)
{
    Q_ASSERT(data.size()==4);
    quint16 x=0, y=0, z=0;
    bool ok = (data[0]==0)?true:false;
    if (ok) {
        x = _1byte2int(data.mid(1, 1));
        y = _1byte2int(data.mid(2, 1));
        z = _1byte2int(data.mid(3, 1));
    }
    qDebug() << "Stop calibration:" << QString::number((quint8)data[0], 16)
            << QString::number((quint8)data[1], 16)
            << QString::number((quint8)data[2], 16)
            << QString::number((quint8)data[3], 16);
    emit StopCalibration(ok, x, y, z);
}

void HDM50Protocol::parse_save_calibration(const QByteArray& data)
{
    Q_ASSERT(data.size()==1);
    bool ok = (data[0]==0)?true:false;
    emit SaveCalibration(ok);
}

QByteArray HDM50Protocol::cmd_set_declination(qreal declination) const
{
    QByteArray set_declination(_set_declination, sizeof(_set_declination));
    qreal fractpart, intpart;
    if (declination < -99.9) {
        declination = -99.9;
    } else if (declination>99.9) {
        declination = 99.9;
    }
    fractpart = modf (declination , &intpart);
    intpart = qAbs(intpart);
    fractpart = qAbs(fractpart);
    unsigned char byte = (unsigned char)(intpart/10);
    byte |= (unsigned char)(declination<0.0?0x10:0x00);
    set_declination[4] = byte;
    byte = ((unsigned char)intpart%10)<<4;
    byte |= 0x0F&qRound(fractpart*10);

    set_declination[5] = byte;
    byte = 0;
    for(int i = 1; i<(set_declination.size()-1); ++i ) {
        byte += set_declination[i];
    }
    set_declination[set_declination.size()-1] = byte;
    return set_declination;
}

QByteArray HDM50Protocol::cmd_set_baud_rate(quint32 bps) const
{
    QByteArray set_baud_rate(_set_baud_rate, sizeof(_set_baud_rate));
    unsigned char byte;
    switch(bps){
        case 2400:
            byte = 0;
            break;
        case 4800:
            byte = 1;
            break;
        case 9600:
            byte = 2;
            break;
        case 19200:
            byte = 3;
            break;
        case 38400:
            byte = 4;
            break;
        case 115200:
            byte = 5;
            break;
        default:
            byte = 2;
            break;
    };
    set_baud_rate[4] = byte;
    byte = 0;
    for (int i = 1; i<(set_baud_rate.size()-1); ++i) {
        byte += set_baud_rate[i];
    }
    set_baud_rate[set_baud_rate.size()-1] = byte;
    return set_baud_rate;
}

QByteArray HDM50Protocol::cmd_set_module_address(quint8 address) const
{
    QByteArray set_module_address(_set_module_address, sizeof(_set_module_address));
    set_module_address[4] = address;
    unsigned char byte = 0;
    for (int i = 1; i<(set_module_address.size()-1); ++i) {
        byte += set_module_address[i];
    }
    set_module_address[set_module_address.size()-1] = byte;
    return set_module_address;
}

QByteArray HDM50Protocol::cmd_set_angle_output_mode(bool auto_mode) const
{
    unsigned char byte = auto_mode?1:0;
    QByteArray set_angle_output_mode(_set_output_mode, sizeof(_set_output_mode));
    set_angle_output_mode[4] = byte;
    byte = 0;
    for (int i = 1; i<(set_angle_output_mode.size()-1); ++i) {
        byte += set_angle_output_mode[i];
    }
    set_angle_output_mode[set_angle_output_mode.size()-1] = byte;
    return set_angle_output_mode;
}

QByteArray HDM50Protocol::cmd_set_mounting_mode(bool horizontal) const
{
    unsigned char byte = horizontal?0:1;
    QByteArray set_mounting_mode(_set_mounting_mode, sizeof(_set_mounting_mode));
    set_mounting_mode[4] = byte;
    byte = 0;
    for (int i = 1; i<(set_mounting_mode.size()-1); ++i) {
        byte += set_mounting_mode[i];
    }
    set_mounting_mode[set_mounting_mode.size()-1] = byte;
    return set_mounting_mode;
}



HDM50Stream::HDM50Stream(QObject* parent):
    QObject(parent), status(FINISHED),
    lost(0), length(-1), address(-1),
    rx(0)
{
    data.clear();
}

void HDM50Stream::reset()
{
    status = FINISHED;
    data.clear();
    length = -1;
    lost = 0;
    address = -1;
}

void HDM50Stream::put(unsigned char byte)
{
//    qDebug() << "Put:" << QString::number(byte, 16);
    if (status==FINISHED) {
        switch(byte) {
            case 0x68:
                data.clear();
                status = START;
                length = lost = 0;
//                qDebug() << "FINISHED->START";
                break;
            default:
                break;
        };
    } else if (status==START) {
        if (length==0) {
            length = (int) byte;
            lost = length-1;
            status = ACCUMULATE;
//            qDebug() << "START->ACCUMULATE" << length << lost;
        }
    } else if (status==ACCUMULATE) {
        if (lost==length-1) {
            address = (int) byte;
            lost -= 1;
//            qDebug() << "ACCUMULATE.1" << length << lost;
        } else if (lost>1) {
            data.append(byte);
            lost -= 1;
//            qDebug() << "ACCUMULATE.2" << length << lost;
        } else if (lost==1) {
//            qDebug() << "ACCUMULATE.3" << length << lost;
            unsigned char sum = 0;
            foreach(char ch, data) {
                sum += (unsigned char)ch;
            }
            sum += (unsigned char)length;
            sum += (unsigned char)address;
            lost -= 1;
            if (sum!=byte) {
                emit Error("Serial stream error: checksum error.");
                reset();
            } else {
//                qDebug() << "new packet ready";
                emit newPacket(address, data);
                status = FINISHED;
            }
        } else {
            emit Error("Serial stream error: data flow analyzer in inconsistent state. To be reset.");
            reset();
        }
    }
    ++rx;
}
