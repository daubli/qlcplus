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
#include <QStringList>
#include <QString>
#include <QDebug>
#include <QThread>

#include "idn.h"
#include "idnconfiguration.h"

static bool initialized = false;

#define MAX_INIT_RETRY 10

/*****************************************************************************
 * Initialization
 *****************************************************************************/

IdnPlugin::~IdnPlugin(){}

void IdnPlugin::init()
{
  foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
  {
      foreach (QNetworkAddressEntry entry, interface.addressEntries()){
        QHostAddress addr = entry.ip();
        if (addr.protocol() != QAbstractSocket::IPv6Protocol)
        {
          IdnOutput tmp;
          tmp.address = entry;
          tmp.controller = NULL;

          bool alreadyInList = false;

          for(int j = 0; j < m_Outputmapping.count(); j++){
            if (m_Outputmapping.at(j).address == tmp.address){
              alreadyInList = true;
              break;
            }
          }
          if (alreadyInList == false){
            m_Outputmapping.append(tmp);
          }
        }
      }
  }
  if(!initialized){
      initialized = true;
      m_settingsFile = "/idn.ini";
      QFileInfo checkFile(QDir::currentPath()+m_settingsFile);
      if(checkFile.exists() && checkFile.isFile()){
        loadSettings();
      }
  }
}

QString IdnPlugin::name()
{
    return QString("IDN");
}

int IdnPlugin::capabilities() const
{
    return QLCIOPlugin::Output | QLCIOPlugin::Infinite;
}

/*****************************************************************************
 * Open/close
 *****************************************************************************/

bool IdnPlugin::openOutput(quint32 output, quint32 universe)
{   
  if (requestLine(output, MAX_INIT_RETRY) == false)
        return false;
  
  if(m_Outputmapping[output].controller == NULL){
    IdnController *controller = new IdnController(m_Outputmapping.at(output).address,
                                                  output, m_manualClients, this);
    m_Outputmapping[output].controller = controller;
  }

  m_Outputmapping[output].controller->addUniverse(universe);

  addToMap(universe, output, Output);
  return true;
}

bool IdnPlugin::requestLine(quint32 line, int retries)
{
    int retryCount = 0;

    while (line >= (quint32)m_Outputmapping.length())
    {
        qDebug() << "[IDN] cannot open line" << line << "(available:" << m_Outputmapping.length() << ")";
		QThread::sleep(1);
        init();
        if (retryCount++ == retries)
            return false;
    }
    return true;
}

void IdnPlugin::closeOutput(quint32 output, quint32 universe)
{
  if (output >= (quint32)m_Outputmapping.length())
      return;
  
  removeFromMap(output, universe, Output);
  IdnController *controller = m_Outputmapping.at(output).controller;
  if(controller != NULL){
    if(controller->closeByUniverse(universe)){
      delete m_Outputmapping[output].controller;
      m_Outputmapping[output].controller = NULL; 
    }
  }
}

/*****************************************************************************
 * Fill Outputlist
 *****************************************************************************/

QStringList IdnPlugin::outputs()
{
    QStringList list;
    int j = 0;

    init();

    foreach(IdnOutput line, m_Outputmapping)
    {
        list << QString("%1: %2").arg(j + 1).arg(line.address.ip().toString());
        j++;
    }
    return list;
}

/*****************************************************************************
 * Plugin- and Output Info
 *****************************************************************************/

QString IdnPlugin::pluginInfo()
{
  QString str;

  str += QString("<HTML>");
  str += QString("<HEAD>");
  str += QString("<TITLE>%1</TITLE>").arg(name());
  str += QString("</HEAD>");
  str += QString("<BODY>");

  str += QString("<P>");
  str += QString("<H3>%1</H3>").arg(name());
  str += tr("This plugin provides DMX output for devices supported by "
            "the IDN driver suite.");
  str += QString("</P>");

  return str;
}

QString IdnPlugin::outputInfo(quint32 output)
{
  Q_UNUSED(output);
    QString str;
    IdnController *controller = m_Outputmapping.at(output).controller;

    if (output != QLCIOPlugin::invalidLine())
    {

            str += QString("<H3>%1 %2</H3>").arg(tr("Output")).arg(outputs()[output]);
            str += QString("<P>");
            if(controller != NULL){
                str += QString("Status: Open");
                str += QString("<BR>");
                str += QString("Packets send: %1").arg(controller->getPacketSentNumber());
                str += QString("<BR>");
            }else{
                str += QString("Status: Not open");
            }
            str += QString("</P>");
    }
    str += QString("</BODY>");
    str += QString("</HTML>");

    return str;
}

/*****************************************************************************
 * Send DMX Data to controller, that builds a IDN Packet and sends the Packet
 *****************************************************************************/

void IdnPlugin::writeUniverse(quint32 universe, quint32 output, const QByteArray &data)
{
  if (output >= (quint32) m_Outputmapping.count())
    return;

  IdnController *controller = m_Outputmapping.at(output).controller;
  if (controller != NULL)
    controller->handleDmx(universe, data);
}

/*****************************************************************************
* Reconfigure Controller after change of settings
*****************************************************************************/
 void IdnPlugin::reconfigureController(){
   //close all open outputs
   for(int i = 0; i < m_Outputmapping.length(); i++){
     closeOutput(i, 0);
   }
}

/*****************************************************************************
 * Configuration
 ****************************************************************************/

void IdnPlugin::configure(){
  IdnConfiguration config(this);
  config.exec();
  //reconfigureController();
  emit configurationChanged();
}

bool IdnPlugin::canConfigure(){
    return true;
}

QList<IdnOutput> IdnPlugin::getOutputmapping()
{
    return m_Outputmapping;
}

//ToDo
//void IdnPlugin::setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value){
//    if (line >= (quint32)m_Outputmapping.length())
//            return;

//    IdnController *controller = m_Outputmapping.at(line).controller;
//    if (controller == NULL)
//        return;

//    if(type == Output){
//        qWarning() << Q_FUNC_INFO << name << "is not a valid IDN parameter";
//    }
//    QLCIOPlugin::setParameter(universe, line, type, name, value);
//}

/*****************************************************************************
 * Load Settings from idn.ini File
 *****************************************************************************/

QHash<QPair<int, QHostAddress>, IdnSettings> IdnPlugin::loadSettings(){
  QSettings settings(QDir::currentPath()+m_settingsFile, QSettings::IniFormat);
  QString groupname;
  for(int i = 0; i < IDN_MAX_CLIENTS; i++){
    groupname = "receiver" + QString::number(i+1);
    settings.beginGroup(groupname);
    QString ip = settings.value("ip", "0").toString();
    if(QString::compare(ip, "0") != 0){
      QHostAddress clientIP(ip);
      if(m_manualClients.contains(qMakePair(settings.value("port, 7255").toInt()*1000+settings.value("channel", "0").toInt(), clientIP)) == false)
      {
        IdnSettings newClient;
        newClient.port = settings.value("port", "7255").toInt();
        newClient.universe = settings.value("universe", "1").toInt();
        newClient.rangeBegin = settings.value("rangeBegin", "1").toInt();
        newClient.rangeEnd = settings.value("rangeEnd", "1").toInt();
        newClient.mode = settings.value("mode", "4").toInt();
        newClient.interface = QHostAddress(settings.value("interface", "127.0.0.1").toString());
        newClient.idnChannel = settings.value("channel", "0").toInt();
		newClient.scan = false;
        m_manualClients[qMakePair(newClient.port*1000+newClient.idnChannel, clientIP)] = newClient;
      }
    }
    settings.endGroup();
  }
  return m_manualClients;
}

void IdnPlugin::setSetting(QHash<QPair<int, QHostAddress>, IdnSettings> settings){
    m_manualClients.clear();
    m_manualClients = settings;
}

QHash<QPair<int, QHostAddress>, IdnSettings> IdnPlugin::getSetting(){
  return m_manualClients;
}

///*****************************************************************************
// * Initializes the ini-File (if its not exists yet)
// *****************************************************************************/

// void IdnPlugin::initSettingsFile(){
//   QSettings settings(QDir::currentPath()+m_settingsFile, QSettings::IniFormat);
//   for(int i = 1; i < MAX_OUTPUT+1; i++){
//    settings.beginGroup("receiver"+QString::number(i));
//    settings.setValue("ip", "0");
//    settings.setValue("interface", "127.0.0.1");
//    settings.setValue("port", "7255");
//    settings.setValue("rangeBegin", "1");
//    settings.setValue("rangeEnd", "512");
//    settings.setValue("mode", "4");
//    settings.setValue("universe", "1");
//    settings.endGroup();
//   }
// }

/*****************************************************************************
 * Plugin export
 ****************************************************************************/
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(idn, IdnPlugin)
#endif
