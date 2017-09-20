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

#include <QDateTime>
#include <QThread>
#include <QtAlgorithms>
#include <QList>
#include <QPair>
#include <QMap>
#include <QFileInfo>
#include <QSettings>
#include <QHash>
#include <QHashIterator>
#include <QMutableHashIterator>
#include <QTextStream>

#include "idnclient.h"
#include "idncontroller.h"

IdnController::IdnController(QNetworkAddressEntry const& address, quint32 line, QHash<QPair<int, QHostAddress>, IdnSettings> settings, QObject *parent)
    : QObject(parent)
    , m_ipAddr(address.ip())
    , m_packetSent(0)
    , m_line(line)
    , m_socket(new QUdpSocket(this))
    , m_packetizer(new IdnPacketizer())
    , m_fileSettings(settings)
{
	m_broadcastAddr = address.broadcast();
	m_prefixLength = address.prefixLength();
	m_scanSocket = new QUdpSocket(this);

	m_socket->bind(m_ipAddr, 0);
	initClients();
}

IdnController::~IdnController()
{
}

int IdnController::countVirtualClients(QHostAddress address, int port){
    QHashIterator<QPair<int, QHostAddress>, IdnClientInfo> client(getClientsList());
    int count = 0;
    while(client.hasNext()){
        client.next();
        if(client.key().second == address && client.value().port == port){
            count++;
        }
    }
    return count;
}

void IdnController::initClients(){
    QHashIterator<QPair<int, QHostAddress>, IdnSettings> settings(m_fileSettings);
	while(settings.hasNext()){
		settings.next();
        if(countVirtualClients(settings.key().second, settings.value().port) == 0){
            sequenceNumbers.insert(qMakePair(settings.key().second, settings.value().port), 0);
        }
		IdnClientInfo client;
		client.port = settings.value().port;
		client.mode = settings.value().mode;
        client.idnChannel = settings.value().idnChannel;
		client.rangeBegin = settings.value().rangeBegin;
		client.rangeEnd = settings.value().rangeEnd;
		client.universe = settings.value().universe;
		client.interface = settings.value().interface;
        client.client = new IdnClient(settings.key().second, m_socket, client.port,
                                            client.rangeBegin, client.rangeEnd, client.mode, client.idnChannel, &sequenceNumbers);
		m_clientsList[settings.key()] = client;
	}
}

/*****************************************************************************
 * Checks wheater a config packet is necessary and calls the Packetizer to build
   the packet. Then the function sends the packet to the specific receiver. 
 *****************************************************************************/
void IdnController::handleDmx(const quint32 universe, const QByteArray &data)
{	
    QHashIterator<QPair<int, QHostAddress>, IdnClientInfo> client(getClientsList());
    quint64 t_packetSend = 0;
    while(client.hasNext()){
		client.next();
        if(client.value().universe-1 == universe && client.value().interface == m_ipAddr){
			IdnClient *c = (IdnClient*)client.value().client; 
			c->sendDmx(data);
            client.value().client->sendDmx(data);
            t_packetSend += client.value().client->getPacketSentNumber();
		}
	}
    m_packetSent = t_packetSend;
}

QString IdnController::getNetworkIP()
{
    return m_ipAddr.toString();
}

QHash<QPair<int, QHostAddress>, IdnClientInfo> IdnController::getClientsList()
{
    return m_clientsList;
}

void IdnController::addUniverse(quint32 universe)
{
    qDebug() << "[IDN] addUniverse - universe" << universe;
    if (!m_universeMap.contains(universe))
    {
        UniverseInfo info;
        info.outputAddress = m_broadcastAddr;
        info.outputUniverse = universe;
        m_universeMap[universe] = info;
    }
    //hier muss der Scan nach Devices aktiviert werden
}

void IdnController::removeUniverse(quint32 universe)
{
    if (m_universeMap.contains(universe))
    {
        m_universeMap.take(universe);
    }
}

QList<quint32> IdnController::universesList()
{
    return m_universeMap.keys();
}

UniverseInfo *IdnController::getUniverseInfo(quint32 universe)
{
    if (m_universeMap.contains(universe))
        return &m_universeMap[universe];

    return NULL;
}

quint32 IdnController::line()
{
    return m_line;
}

quint64 IdnController::getPacketSentNumber()
{
    return m_packetSent;
}

bool IdnController::closeByUniverse(quint32 universe){
    QMutableHashIterator<QPair<int, QHostAddress>, IdnClientInfo> clients(m_clientsList);
	while(clients.hasNext()){
		clients.next();
		if(clients.value().universe-1 == universe){
			delete clients.value().client;
			clients.remove();
		}
	}
	return m_clientsList.isEmpty();
}
