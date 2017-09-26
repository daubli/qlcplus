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

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QMenu>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>

#include "idnconfiguration.h"
#include "idn.h"

IdnConfiguration::IdnConfiguration(IdnPlugin *plugin, QWidget *parent) :
    QDialog(parent)
  , m_packetizer(new IdnPacketizer())
{
	Q_ASSERT(plugin != NULL);
    m_plugin = plugin;
    validConfiguration = true;
    setupUi(this);
    uiSettings();
    fillTree();
}

IdnConfiguration::~IdnConfiguration()
{
}

void IdnConfiguration::uiSettings(){
    m_clientTree->setColumnWidth(0, 140); //Iface
    m_clientTree->setColumnWidth(1, 60); //Universe
    m_clientTree->setColumnWidth(3, 70); //Port
    m_clientTree->setColumnWidth(4, 250); //Mode
    m_clientTree->setColumnWidth(5, 80); //Channel ID
    m_clientTree->setColumnWidth(6, 80); //Range Begin
    m_clientTree->setColumnWidth(7, 80); //Range End
}


void IdnConfiguration::fillTree(){
    QHash<QPair<int, QHostAddress>, IdnSettings> clientMap = m_plugin->getSetting();
    QHashIterator<QPair<int, QHostAddress>, IdnSettings> it(clientMap);

    while(it.hasNext()){
        it.next();
        QTreeWidgetItem* pitem = new QTreeWidgetItem(m_clientTree);

        QSpinBox *universeSpin = new QSpinBox(this);
        universeSpin->setMinimum(1);
        universeSpin->setValue(it.value().universe);
        m_clientTree->setItemWidget(pitem, 1, universeSpin);

		if (it.value().scan){
			pitem->setText(0, it.value().interface.toString());
            pitem->setText(2, it.key().second.toString());
			pitem->setText(3, QString::number(it.value().port));
		}
		else{
			QComboBox *ifaceCombo = new QComboBox(this);
			foreach(IdnOutput output, m_plugin->getOutputmapping()){
				ifaceCombo->addItem(output.address.ip().toString());
			}
			ifaceCombo->setCurrentText(it.value().interface.toString());
			m_clientTree->setItemWidget(pitem, 0, ifaceCombo);

            QLineEdit *ipAddrEdit = new QLineEdit(it.key().second.toString(), this);
			m_clientTree->setItemWidget(pitem, 2, ipAddrEdit);
			
			QSpinBox *portSpin = new QSpinBox(this);
			portSpin->setRange(1000, 10000);
			portSpin->setValue(it.value().port);
			m_clientTree->setItemWidget(pitem, 3, portSpin);
		}

        QComboBox *combo = new QComboBox(this);
        combo->addItem(tr("Laser Projector Effects (Discrete)"));
        combo->addItem(tr("Laser Projector Effects (Discrete) - Optimized"));
        combo->addItem(tr("DMX512 (Discrete)"));
        combo->addItem(tr("DMX512 (Discrete) - Optimized"));

        switch(it.value().mode){
            case 4:
                combo->setCurrentIndex(0);
                break;
            case 5:
                combo->setCurrentIndex(1);
                break;
            case 6:
                combo->setCurrentIndex(2);
                break;
            case 7:
                combo->setCurrentIndex(3);
                break;
        }
        m_clientTree->setItemWidget(pitem, 4, combo);

        QSpinBox *idnChannelSpinBox = new QSpinBox(this);
        idnChannelSpinBox->setRange(0, 63);
        idnChannelSpinBox->setValue(it.value().idnChannel);
        m_clientTree->setItemWidget(pitem, 5, idnChannelSpinBox);

        QSpinBox *rangeBeginEdit = new QSpinBox(this);
        rangeBeginEdit->setRange(1,512);
        rangeBeginEdit->setValue(it.value().rangeBegin);
        m_clientTree->setItemWidget(pitem, 6, rangeBeginEdit);

        QSpinBox *rangeEndEdit = new QSpinBox(this);
        rangeEndEdit->setRange(1,512);
        rangeEndEdit->setValue(it.value().rangeEnd);
        m_clientTree->setItemWidget(pitem, 7, rangeEndEdit);
    }
}

short IdnConfiguration::validateInput(QString ipAddress, int port, unsigned short channel, unsigned short rangeBegin, unsigned short rangeEnd){
    QTreeWidgetItemIterator it(m_clientTree);
    QList<quint8> candidateList;

    for(int i = 0; i < m_clientTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_clientTree->topLevelItem(i);
        QString wIpAddr;
        if(item->text(2).isEmpty()){
            QLineEdit *ipLineEdit = qobject_cast<QLineEdit*>(m_clientTree->itemWidget(item, 2));
            wIpAddr = ipLineEdit->displayText();
            //case empty
        }else{
            wIpAddr = item->text(2);
        }
        if(QString::compare(wIpAddr, ipAddress) == 0){
            int wPort;
            unsigned short wChannel = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 5))->value();
            if(item->text(3).isEmpty()){
                QSpinBox *portSpinBox = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 3));
                wPort = portSpinBox->value();
            }else{
                wPort = item->text(3).toInt();
            }

            if(wPort == port && wChannel == channel){
                candidateList << 1;
            }else if(wPort == port){
                if(qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 6))->value() >= rangeBegin &&
                        qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 7))->value() <= rangeEnd)
                {
                    return -1;
                }
            }
        }
    }

    return candidateList.count();
}

int IdnConfiguration::exec()
{
    return QDialog::exec();
}

void IdnConfiguration::addReceiverSlot(){
    QTreeWidgetItem* nitem = new QTreeWidgetItem(m_clientTree);

    QComboBox *ifaceCombo = new QComboBox(this);

    foreach(IdnOutput output, m_plugin->getOutputmapping()){
        ifaceCombo->addItem(output.address.ip().toString());
    }
    ifaceCombo->setCurrentIndex(ifaceCombo->count()-1);
    m_clientTree->setItemWidget(nitem, 0, ifaceCombo);

    QSpinBox *universeSpin = new QSpinBox(this);
    universeSpin->setMinimum(1);
    universeSpin->setValue(1);
    m_clientTree->setItemWidget(nitem, 1, universeSpin);

    QLineEdit *ipAddrEdit = new QLineEdit("", this);
    m_clientTree->setItemWidget(nitem, 2, ipAddrEdit);

    QSpinBox *portSpin = new QSpinBox(this);
    portSpin->setRange(1000, 10000);
    portSpin->setValue(IDN_PORT);
    m_clientTree->setItemWidget(nitem, 3, portSpin);

    QComboBox *combo = new QComboBox(this);
    combo->addItem(tr("Laser Projector Effects (Discrete)"));
    combo->addItem(tr("Laser Projector Effects (Discrete) - Optimized"));
    combo->addItem(tr("DMX512 (Discrete)"));
    combo->addItem(tr("DMX512 (Discrete) - Optimized"));
    m_clientTree->setItemWidget(nitem, 4, combo);

    QSpinBox *idnChannelSpinBox = new QSpinBox(this);
    idnChannelSpinBox->setRange(0,63);
    idnChannelSpinBox->setValue(0);
    m_clientTree->setItemWidget(nitem, 5, idnChannelSpinBox);

    QSpinBox *rangeBeginEdit = new QSpinBox(this);
    rangeBeginEdit->setRange(1,512);
    rangeBeginEdit->setValue(1);
    m_clientTree->setItemWidget(nitem, 6, rangeBeginEdit);

    QSpinBox *rangeEndEdit = new QSpinBox(this);
    rangeEndEdit->setRange(1,512);
    rangeEndEdit->setValue(512);
    m_clientTree->setItemWidget(nitem, 7, rangeEndEdit);
}

void IdnConfiguration::on_m_buttonBox_accepted()
{
    QHash<QPair<int, QHostAddress>, IdnSettings> settings;
    for(int i = 0; i < m_clientTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_clientTree->topLevelItem(i);
        IdnSettings tmp;

        if(item->text(0).isEmpty()){
            QComboBox *ifaceCombo = qobject_cast<QComboBox*>(m_clientTree->itemWidget(item, 0));
            tmp.interface = QHostAddress(ifaceCombo->currentText());
			tmp.scan = false;
        }else{
            tmp.interface = QHostAddress(item->text(0));
			tmp.scan = true;
        }

        QSpinBox *universeSpinBox = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 1));
        tmp.universe = universeSpinBox->value();

        QString ipStr;
        if(item->text(2).isEmpty()){
            QLineEdit *ipAddrEdit = qobject_cast<QLineEdit*>(m_clientTree->itemWidget(item, 2));
            ipStr = ipAddrEdit->text();
        }else{
            ipStr = item->text(2);
        }
        QHostAddress ip = QHostAddress(ipStr);

        if(ip.isNull()){
            QMessageBox::critical(this, tr("QLC+ IDN Plugin"), tr("You have to enter an valid IP-Address.\n"), QMessageBox::Cancel);
            return;
        }

        if(item->text(3).isEmpty()){
            QSpinBox *portSpinBox = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 3));
            tmp.port = portSpinBox->value();
        }else{
            tmp.port = item->text(3).toInt();
        }

        QComboBox *modeCombo = qobject_cast<QComboBox*>(m_clientTree->itemWidget(item, 4));
        tmp.mode = modeCombo->currentIndex()+4;

        QSpinBox *idnChannelSpinBox = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 5));
        tmp.idnChannel = idnChannelSpinBox->value();

        QSpinBox *rangeBeginSpinBox = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 6));
        tmp.rangeBegin = rangeBeginSpinBox->value();

        QSpinBox *rangeEndSpinBox = qobject_cast<QSpinBox*>(m_clientTree->itemWidget(item, 7));
        tmp.rangeEnd = rangeEndSpinBox->value();

        short validate = validateInput(ip.toString(), tmp.port, tmp.idnChannel, tmp.rangeBegin, tmp.rangeEnd);

        if(validate > 1){
            QMessageBox::critical(this, tr("QLC+ IDN Plugin"), tr("There are at least two receivers with the same IP-Addres, Port and IDN Channel.\n"), QMessageBox::Cancel);
            return;
        }
        if(validate == -1){
            QMessageBox::critical(this, tr("QLC+ IDN Plugin"), tr("There are at least two receivers with the same IP-Address and Port which have not disjunct DMX channel sets.\n"), QMessageBox::Cancel);
            return;
        }
        settings[qMakePair(64*tmp.port+tmp.idnChannel, ip)] = tmp;
    }

    m_plugin->setSetting(settings);

    QDialog::accept();
}

void IdnConfiguration::deleteSlot(){
    QList<QTreeWidgetItem *> itemList;
    itemList = m_clientTree->selectedItems();
    foreach(QTreeWidgetItem *item, itemList)
    {
       int index = m_clientTree->indexOfTopLevelItem(item);
       m_clientTree->takeTopLevelItem(index);
    }
}

void IdnConfiguration::on_m_buttonBox_rejected()
{
    QDialog::reject();
}

void IdnConfiguration::on_m_scanButton_clicked()
{
    foreach(IdnOutput output, m_plugin->getOutputmapping()){
        QUdpSocket *scanSocket = new QUdpSocket(this);
        scanSocket->bind(output.address.ip(), IDN_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
        sendScan(scanSocket, output.address.ip());
        connect(scanSocket, SIGNAL(readyRead()), this, SLOT(waitforScanReply()));
    }
}

void IdnConfiguration::sendScan(QUdpSocket *scanSocket, QHostAddress outputIP)
{
    //m_scanSocket->bind(IDN_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    QByteArray scanRequestPacket;
    m_packetizer->generateScanRequestPacket(scanRequestPacket);
    qint64 sent = scanSocket->writeDatagram(scanRequestPacket, outputIP.Broadcast, IDN_PORT);
    if (sent < 0){
        qWarning() << "sendScanRequest failed";
        qWarning() << "Errno: " << scanSocket->error();
        qWarning() << "Errmgs: " << scanSocket->errorString();
    }
}

void IdnConfiguration::waitforScanReply()
{
    QUdpSocket *scanSocket = qobject_cast<QUdpSocket*>(sender());
    QByteArray datagram;
    QHostAddress senderAddress;
    while (scanSocket->hasPendingDatagrams())
    {
        datagram.resize(scanSocket->pendingDatagramSize());
        scanSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress);
        if(m_packetizer->validateReply(datagram) && validateInput(senderAddress.toString(), IDN_PORT, 0, 1, 512) < 1){
            QTreeWidgetItem* nitem = new QTreeWidgetItem(m_clientTree);
            nitem->setText(0, scanSocket->localAddress().toString());

            QSpinBox *universeSpin = new QSpinBox(this);
            universeSpin->setMinimum(1);
            universeSpin->setValue(1);
            m_clientTree->setItemWidget(nitem, 1, universeSpin);

            nitem->setText(2, senderAddress.toString());
            nitem->setText(3, QString::number(IDN_PORT));

            QComboBox *combo = new QComboBox(this);
            combo->addItem(tr("Laser Projector Effects (Discrete)"));
            combo->addItem(tr("Laser Projector Effects (Discrete) - Optimized"));
            combo->addItem(tr("DMX512 (Discrete)"));
            combo->addItem(tr("DMX512 (Discrete) - Optimized"));
            m_clientTree->setItemWidget(nitem, 4, combo);

            QSpinBox *idnChannelSpinBox = new QSpinBox(this);
            idnChannelSpinBox->setRange(0, 63);
            idnChannelSpinBox->setValue(0);
            m_clientTree->setItemWidget(nitem, 5, idnChannelSpinBox);

            QSpinBox *rangeBeginEdit = new QSpinBox(this);
            rangeBeginEdit->setRange(1,512);
            rangeBeginEdit->setValue(1);
            m_clientTree->setItemWidget(nitem, 6, rangeBeginEdit);

            QSpinBox *rangeEndEdit = new QSpinBox(this);
            rangeEndEdit->setRange(1,512);
            rangeEndEdit->setValue(512);
            m_clientTree->setItemWidget(nitem, 7, rangeEndEdit);
        }

        //validate reply
        //insert into datastructure
    }
}

void IdnConfiguration::on_m_clientTree_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    if(m_clientTree->selectedItems().count() != 0){
        QAction *deleteClient = new QAction("Delete", this);
        connect(deleteClient, SIGNAL(triggered()), SLOT(deleteSlot()));
        menu->addAction(deleteClient);
        menu->popup(m_clientTree->mapToGlobal(pos));
    }else{
        QAction *scan = new QAction("Scan", this);
        connect(scan, SIGNAL(triggered()), SLOT(on_m_scanButton_clicked()));
        menu->addAction(scan);
        QAction *addClient = new QAction("Add new IDN Client", this);
        menu->addAction(addClient);
        connect(addClient, SIGNAL(triggered()), SLOT(addReceiverSlot()));
        menu->popup(m_clientTree->mapToGlobal(pos));
    }
}

void IdnConfiguration::on_m_addClientButton_clicked()
{
    addReceiverSlot();
}
