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
#ifndef IDNCONTROLLER_H
#define IDNCONTROLLER_H

#include <QObject>
#include <QtNetwork>
#include <QScopedPointer>
#include <QTimer>

#include "idnclient.h"

//INTERVAL FOR CONFIG PACKETS in ms
#define IDN_CONFIG_INTERVAL 180
#define IDN_TIMEOUT 250
#define IDN_SCAN_TIMEOUT 1000
#define IDN_PORT 7255
#define IDN_MAX_CLIENTS 8

typedef struct
{
    QHostAddress outputAddress;
    ushort outputUniverse;
} UniverseInfo;

typedef struct
{
    int port;
    int mode;
    int idnChannel;
    int rangeBegin;
    int rangeEnd;
    quint32 universe;
    QHostAddress interface;
	bool scan;
}IdnSettings;

typedef struct
{
    int port;
    int mode;
    int idnChannel;
    int rangeBegin;
    int rangeEnd;
    quint32 universe;
    QHostAddress interface;
    IdnClient *client;
}IdnClientInfo;

class IdnController : public QObject
{
    Q_OBJECT

public:
  IdnController(QNetworkAddressEntry const& address, quint32 line, QHash<QPair<int, QHostAddress>, IdnSettings> settings, QObject *parent = 0);

	//IdnController(QNetworkAddressEntry const& address, QSharedPointer<QUdpSocket> const& udpSocket, int const& port, int const& begin, int const& end, int const& mode, QObject *parent = 0);
	~IdnController();

	/** Send DMX data to a specific port/universe */
  void handleDmx(const quint32 universe, const QByteArray& data);
  
  /** Return the controller IP address */
  QString getNetworkIP();
  
  /** Returns the map of all clients */
  QHash<QPair<int, QHostAddress>, IdnClientInfo> getClientsList();

  /** Add a universe to the map of this controller */
  void addUniverse(quint32 universe);

  /** Remove a universe from the map of this controller */
  void removeUniverse(quint32 universe);

  /** Return the list of the universes handled by this controller */
  QList<quint32> universesList();

  /** Return the specific information for the given universe */
  UniverseInfo *getUniverseInfo(quint32 universe);

  /** Return the plugin line associated to this controller */
  quint32 line();

  /** Get the number of packets sent by this controller */
  quint64 getPacketSentNumber();
    // Debug
  void DBG_CheckSettings();

  bool closeByUniverse(quint32 universe);
  int countVirtualClients(QHostAddress address, int port);

private:
  /** The network interface associated to this controller */
  QNetworkInterface m_interface;
  QNetworkAddressEntry m_address;
  int m_prefixLength;
  /** The controller IP address as QHostAddress */
  QHostAddress m_ipAddr;
  
  /** The controller broadcast address as QHostAddress */
  QHostAddress m_broadcastAddr;

  /** Counter for transmitted packets */
  quint64 m_packetSent;

  /** Sequence numbers */
  QHash<QPair<QHostAddress, int>, quint32> sequenceNumbers;

  /** QLC+ line to be used when emitting a signal */
  quint32 m_line;

  /** The UDP socket used to send/receive IDN packets */
  QSharedPointer<QUdpSocket> m_socket;
  QUdpSocket *m_scanSocket;

  /** Helper class used to create or parse IDN packets */
  QScopedPointer<IdnPacketizer> m_packetizer;

  /** Map of the IDN clients discovered */
  QHash<QPair<int, QHostAddress>, IdnClientInfo> m_clientsList;
  QHash<QPair<int, QHostAddress>, IdnSettings> m_fileSettings;

  /** Keeps the current dmx values to send only the ones that changed */
  /** It holds values for all the handled universes */
  QMap<int, QByteArray *> m_dmxValuesMap;

  /** Map of the QLC+ universes transmitted/received by this
   *  controller, with the related, specific parameters */
  QMap<quint32, UniverseInfo> m_universeMap;

  /** Mutex to handle the change of output IP address or in general
    *  variables that could be used to transmit/receive data */
  QMutex m_dataMutex;

  QMutex m_seqnumMutex;

  QString m_settingsFile;

 public:
  void sendScan();

private:

  void initClients();
};

#endif
