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

#ifndef IDN_H
#define IDN_H

#include <QString>
#include <QSettings>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QHostAddress>

#include "idncontroller.h"
#include "qlcioplugin.h"

#define IDN_MAX_CLIENTS 8

typedef struct
{
    QNetworkAddressEntry address;
    IdnController* controller;
}IdnOutput;

#define MAX_OUTPUT 8

class IdnPlugin : public QLCIOPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCIOPlugin)
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID QLCIOPlugin_iid)
#endif

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /** @reimp */
    virtual ~IdnPlugin();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /** @reimp */
    int capabilities() const;

    /** @reimp */
    QString pluginInfo();

    bool requestLine(quint32 line, int retries);

    /*********************************************************************
     * Outputs
     *********************************************************************/
public:
    /** @reimp */
    bool openOutput(quint32 output, quint32 universe);

    /** @reimp */
    void closeOutput(quint32 output, quint32 universe);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString outputInfo(quint32 output);

    /** @reimp */
    void writeUniverse(quint32 universe, quint32 output, const QByteArray& data);

    /*********************************************************************
     * Settings
     *********************************************************************/
private:
      /** Map of the IDN clients discovered */
    QHash<QPair<int, QHostAddress>, IdnSettings> m_manualClients;
    QHash<QPair<int, QHostAddress>, IdnSettings> loadSettings();
    //void initSettingsFile();
    QString m_settingsFile;

    //bool initialized;


     /*********************************************************************
     * Mapping
     *********************************************************************/
private:
    /** Map of the IDN plugin Output lines */
    QList<IdnOutput> m_Outputmapping;
    void reconfigureController();


    /**********************************************************************
     * Configuration
     **********************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

    //ToDo
    //    /** @reimp */
    //    void setParameter(quint32 universe, quint32 line, Capability type, QString name, QVariant value);

    QList<IdnOutput> getOutputmapping();

    /** getter Method for configuration */
    QHash<QPair<int, QHostAddress>, IdnSettings> getSetting();
    void setSetting(QHash<QPair<int, QHostAddress>, IdnSettings> settings);
    void sendScanRequest(QUdpSocket *socket);
};

#endif
