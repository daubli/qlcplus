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
#include "idncontroller.h"
#include "idnclient.h"

IdnClient::IdnClient(QHostAddress const &clientAddress, QSharedPointer<QUdpSocket> const& udpSocket,
                     int const& port, int const& rangeBegin, int const& rangeEnd,
                     int const& mode, int const& channelID, QHash<QPair<QHostAddress, int>, quint32> *sequenceNumbers)
  : m_address(clientAddress)
  , m_udpSocket(udpSocket)
  , m_port(port)
  , m_rangeBegin(rangeBegin)
  , m_rangeEnd(rangeEnd)
  , m_mode(mode)
  , m_channelID(channelID)
  , m_packetizer(new IdnPacketizer())
  , m_optimizer(new IdnOptimizer())
  , m_seqnum(0)
  , m_packetSent(0)
  , m_sequenceNumbers(sequenceNumbers)
{
    //first packet has to be a config packet
  config = true;
  //init timestamp
  timestamp = QDateTime::currentMSecsSinceEpoch();
  lastsend = 0;
  blackCounter = 0;

  if (m_rangeBegin < 1){
	  m_rangeBegin = 1;
  }
  if (m_rangeEnd > 512){
	  m_rangeEnd = 512;
  }
  //timer to close the connection in case of a blackout
  closeTimer = new QTimer(this);
  closeTimer->setInterval(IDN_TIMEOUT);
  closeTimer->setSingleShot(true);
  connect(closeTimer, SIGNAL(timeout()), this, SLOT(sendClosePacket()));
}

IdnClient::~IdnClient()
{
	sendClosePacket();
}

void IdnClient::sendClosePacket(){
  QMutexLocker mutex(&m_dataMutex);
  QByteArray dmxPacket;
  m_packetizer->generateClosePacket(dmxPacket, m_seqnum, m_channelID);
  qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
  lastsend = QDateTime::currentMSecsSinceEpoch();
  if (sent < 0){
    qWarning() << "sendDmx failed";
    qWarning() << "Errno: " << m_udpSocket->error();
    qWarning() << "Errmgs: " << m_udpSocket->errorString();
  }else{
    m_packetSent++;
    m_sequenceNumbers->insert(qMakePair(m_address, m_port), ++m_seqnum);
  }
}

QByteArray IdnClient::optimizedMode(const QByteArray &data){
  bool configTime = (QDateTime::currentMSecsSinceEpoch() - timestamp) > IDN_CONFIG_INTERVAL ? true : false;

  QList<QPair<int, int> > ranges = m_optimizer->optimize(data, configTime);
  //setze den Datachunk zusammen
  QByteArray dmxPacket, justifiedData;
  bool addRangeEndFrame = true;

  QMutableListIterator<QPair<int, int> > rangeIterator(ranges);


  while(rangeIterator.hasNext()){
      rangeIterator.next();
      if(rangeIterator.value().first+1 < m_rangeBegin && rangeIterator.value().second+1 >= m_rangeBegin){
          rangeIterator.value().first = m_rangeBegin-1;
      }
      if(rangeIterator.value().first+1 < m_rangeBegin && rangeIterator.value().second+1 < m_rangeBegin){
          rangeIterator.remove();
          continue;
      }
      if(rangeIterator.value().first+1 > m_rangeEnd){
          rangeIterator.remove();
          continue;
      }
      if(rangeIterator.value().second+1 > m_rangeEnd){
          rangeIterator.value().second = m_rangeEnd-1;
      }

    for(int j = rangeIterator.value().first; j < rangeIterator.value().second+1; j++){
      if(rangeIterator.value().first+1 == m_rangeEnd || rangeIterator.value().second+1 == m_rangeEnd){
        addRangeEndFrame = false;
      }
      justifiedData.append(data[j]);
    }
  }

  if(addRangeEndFrame){
    if(data.size() >= m_rangeEnd){
      justifiedData.append(data[m_rangeEnd-1]);
    }
    else{
      justifiedData.append((char)0x0);
    }
    QPair<int, int> finalFrame;
    finalFrame.first = m_rangeEnd-1;
    finalFrame.second = m_rangeEnd-1;
    ranges.append(finalFrame);
  }

  bool newConfig = false;
  if(ranges.length() != oldranges.length()){
    newConfig = true;
  }else{
    for(int i = 0; i < ranges.length(); i++){
      if(ranges[i] == oldranges[i]){
        continue;
      }else{
        newConfig = true;
        break;
      }
    }
  }
  if(!newConfig){
    config = false;
    if(configTime){
      config = true;
    }else{
      config = false;
    }
  }else{
    config = true;
  }

  oldData = justifiedData;
  oldranges = ranges;
  m_packetizer->setupIdnDmx(dmxPacket, m_mode, m_channelID, justifiedData, ranges, m_seqnum, config);

  if(config)
    timestamp = QDateTime::currentMSecsSinceEpoch();
  return dmxPacket; 
}

QByteArray IdnClient::rangeMode(const QByteArray &data){
  QByteArray dmxPacket;
  QByteArray universeRange(m_rangeEnd, (char)0x0);
    universeRange.replace(0, data.length(), data);
    QByteArray justifiedData = universeRange.mid(m_rangeBegin-1, m_rangeEnd-m_rangeBegin+1);

  if((QDateTime::currentMSecsSinceEpoch() - timestamp) > IDN_CONFIG_INTERVAL || m_packetSent == 0){
    config = true;
  }else{
    config = false;
  }
  m_seqnum = m_sequenceNumbers->find(qMakePair(m_address, m_port)).value();
  m_packetizer->setupIdnDmx(dmxPacket, m_mode, m_channelID, justifiedData, m_rangeBegin, m_seqnum, config);

  if(config)
    timestamp = QDateTime::currentMSecsSinceEpoch();
  
  return dmxPacket;   
}

/*****************************************************************************
 * Checks wheater a config packet is necessary and calls the Packetizer to build
   the packet. Then the function sends the packet to the specific receiver. 
 *****************************************************************************/
void IdnClient::sendDmx(const QByteArray &data){
  QMutexLocker mutex(&m_dataMutex);
  m_seqnum = m_sequenceNumbers->find(qMakePair(m_address, m_port)).value();

  if(closeTimer->isActive()){
    closeTimer->stop();
  }

  QByteArray dmxPacket;
  if(m_seqnum == 0){
      m_packetizer->generateNullPacket(dmxPacket, m_seqnum, m_mode, m_channelID);
      qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
      lastsend = QDateTime::currentMSecsSinceEpoch();
        
      if (sent < 0){
          qWarning() << "sendDmx failed";
          qWarning() << "Errno: " << m_udpSocket->error();
        qWarning() << "Errmgs: " << m_udpSocket->errorString();
      }else{
        m_sequenceNumbers->insert(qMakePair(m_address, m_port), ++m_seqnum);
        m_packetSent++;
      }
  }

  if(m_mode == 5 || m_mode == 7){
    quint64 td = QDateTime::currentMSecsSinceEpoch() - lastsend;
    if((QDateTime::currentMSecsSinceEpoch() - lastsend) > (int)(140+(44*(m_rangeEnd)))/1000){
        dmxPacket = optimizedMode(data);
      qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);

      if (sent < 0){
        qWarning() << "sendDmx failed";
        qWarning() << "Errno: " << m_udpSocket->error();
        qWarning() << "Errmgs: " << m_udpSocket->errorString();
      }
      else{
        lastsend = QDateTime::currentMSecsSinceEpoch();
        m_sequenceNumbers->insert(qMakePair(m_address, m_port), ++m_seqnum);
        m_packetSent++;
      }
    }
  }else{
    quint64 td = QDateTime::currentMSecsSinceEpoch() - lastsend;
    if((QDateTime::currentMSecsSinceEpoch() - lastsend) > (int)(140+(44*(m_rangeEnd-m_rangeBegin)))/1000){
      dmxPacket = rangeMode(data);
      qint64 sent = m_udpSocket->writeDatagram(dmxPacket, m_address, m_port);
      
      if (sent < 0){
              qWarning() << "sendDmx failed";
              qWarning() << "Errno: " << m_udpSocket->error();
              qWarning() << "Errmgs: " << m_udpSocket->errorString();
        }
        else{
          lastsend = QDateTime::currentMSecsSinceEpoch();
          m_sequenceNumbers->insert(qMakePair(m_address, m_port), ++m_seqnum);
          m_packetSent++;
        }
    }
  }
}

quint64 IdnClient::getPacketSentNumber(){
    return m_packetSent;
}
