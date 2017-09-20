//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// Copyright (c) 2017, University of Bonn, Institute of Computer Science 4.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the University of Bonn nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the University of Bonn.
//
// Contacts:    Dr. Matthias Frank (matthew@cs.uni-bonn.de)
//              Daniel Schröder    (schroed1@cs.uni-bonn.de)
//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef IDNCLIENT_H
#define IDNCLIENT_H

#include <QObject>
#include <QtNetwork>
#include <QScopedPointer>
#include <QTimer>
#include <QMutex> 

#include "idnoptimizer.h"
#include "idnpacketizer.h"

#define IDN_CONFIG_INTERVAL 180
#define IDN_TIMEOUT 250

class IdnClient : public QObject
{
    Q_OBJECT

public:
    IdnClient(QHostAddress const &clientAddress, QSharedPointer<QUdpSocket> const& udpSocket,
              int const& port, int const& rangeBegin, int const& rangeEnd, int const& mode,
              int const& channelID, QHash<QPair<QHostAddress, int>, quint32> *sequenceNumbers);
    ~IdnClient();
    void sendDmx(const QByteArray &data);
    quint64 getPacketSentNumber();
  private:
    QHostAddress m_ifaceAddr;
    QHostAddress m_address;
    QSharedPointer<QUdpSocket> m_udpSocket;
    quint16 m_port; 
    quint16 m_rangeBegin;
    quint16 m_rangeEnd;
    quint8 m_mode;
    quint8 m_channelID;

    QScopedPointer<IdnPacketizer> m_packetizer;
    QScopedPointer<IdnOptimizer> m_optimizer;

    //////////////////////
    ///For packet processing
    //////////////////////
    /** true = packet with config header - false = packet without config header */
    bool config;
    int blackCounter;

    /** if data does not change the old data will be send */
    QByteArray oldData;
    /** old range list */
    QList<QPair<int, int> >  oldranges;
    /** Timestamp to check wheater a config packet is neccessary */
    qint64 timestamp;
    /** Timestamp of the last submitted packet */
    qint64 lastsend;
    /** Sequence Number */
    quint32 m_seqnum;
	quint32 m_packetSent;
    QHash<QPair<QHostAddress, int>, quint32> *m_sequenceNumbers;

    QMutex m_dataMutex;

    QTimer *closeTimer;

    QByteArray optimizedMode(const QByteArray &data);
    QByteArray rangeMode(const QByteArray &data);
  private slots:
    void sendClosePacket();
};

#endif
