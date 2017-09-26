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

#ifndef IDNCONFIGURATION_H
#define IDNCONFIGURATION_H

#include <QUdpSocket>

#include "ui_idnconfiguration.h"
#include "idnpacketizer.h"

class IdnPlugin;

class IdnConfiguration : public QDialog, public Ui_IdnConfiguration
{
    Q_OBJECT

public:
    IdnConfiguration(IdnPlugin *plugin, QWidget *parent = 0);
    virtual ~IdnConfiguration();

public slots:
    int exec();
    void addReceiverSlot();

private slots:
    void on_m_buttonBox_accepted();

    void on_m_buttonBox_rejected();

    void on_m_scanButton_clicked();

    void waitforScanReply();

    void on_m_clientTree_customContextMenuRequested(const QPoint &pos);

    void deleteSlot();

    void on_m_addClientButton_clicked();

private:
    //QHash<QHostAddress, QUdpSocket> m_scanSockets;
    IdnPacketizer *m_packetizer;
    void fillTree();
    short validateInput(QString ipAddress, int port, unsigned short channel, unsigned short rangeBegin, unsigned short rangeEnd);
    void uiSettings();
    void settings();
    bool validConfiguration;

    void sendScan(QUdpSocket *scanSocket, QHostAddress outputIP);

private:
	IdnPlugin* m_plugin;
private:
    Ui::IdnConfiguration *ui;
};

#endif // IDNCONFIGURATION_H
