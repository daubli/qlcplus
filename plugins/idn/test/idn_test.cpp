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

#include <QTest>

#define private public
#include "idn_test.h"
#include "../src/idnpacketizer.h"
#undef private

/****************************************************************************
 * IDN tests
 ****************************************************************************/

void Idn_Test::setupIdn()
{
    IdnPacketizer idnpacketizer;
    QByteArray data;

    //Packet Fundamentals
    const QByteArray idnhello = QByteArray("\x40\x00\x00\x00");
    const QByteArray idnSimpleConfig = QByteArray("\x01\x01\x00\x04");
    const QByteArray idnNoServiceModeConfig = QByteArray("\x00\x01\x00\x04");
    const QByteArray simpleServiceConfig = QByteArray("\x42\x00\x01\x00\x00\x00\x00\x00");
    const QByteArray halfServiceModeConfig = QByteArray("\x43\x00\x02\x00");
    const QByteArray fullServiceModeConfig = QByteArray("\x43\x00\x02\x00");
    const QByteArray empty;
    static const char halfRaw[] = {
        0x3a, 0x08, 0xef, 0xda, 0x7a, 0xcf, 0xf5, 0x87, 0x4f, 0x55, 0x2f, 0x66, 0xf5, 0xb3, 0x86, 0x07, 
        0x39, 0xfb, 0x4b, 0x72, 0x84, 0xf1, 0xee, 0xdd, 0xae, 0x96, 0x55, 0xb9, 0xa7, 0xf3, 0x2e, 0x10, 
        0x8e, 0x8c, 0x4e, 0x43, 0x1e, 0x89, 0x4b, 0x72, 0xca, 0xb3, 0xb6, 0xcd, 0x1d, 0xd4, 0x5a, 0x8c, 
        0x53, 0x75, 0x29, 0x3a, 0xd1, 0x38, 0x19, 0x75, 0x91, 0x72, 0x6d, 0x03, 0xf1, 0xaf, 0x98, 0x9a, 
        0x61, 0xf4, 0x8f, 0xb6, 0xc3, 0xb6, 0x92, 0xc5, 0xa2, 0xc4, 0x19, 0x1e, 0x8c, 0xa4, 0x38, 0x2d, 
        0xb8, 0xf8, 0x13, 0xf4, 0x5c, 0x67, 0x3b, 0x58, 0xa9, 0x26, 0x1a, 0x6e, 0xb5, 0x86, 0x36, 0xcf, 
        0xc4, 0xa2, 0x39, 0x7c, 0x79, 0xf5, 0x08, 0x7c, 0xf9, 0x65, 0x44, 0x36, 0x8d, 0xc7, 0xb4, 0xe0, 
        0x41, 0x32, 0x9e, 0xa4, 0x70, 0x48, 0x32, 0xd4, 0x97, 0xb8, 0x59, 0xf2, 0x21, 0xdd, 0x36, 0x58, 
        0xd2, 0x3b, 0x94, 0x76, 0x39, 0xef, 0x2b, 0xfc, 0x21, 0x2b, 0x48, 0xf1, 0xbf, 0x87, 0x24, 0x93, 
        0xcc, 0x38, 0x79, 0xd1, 0x06, 0x02, 0x12, 0x7a, 0x45, 0x82, 0x81, 0x04, 0x81, 0x89, 0xb7, 0xc7, 
        0x62, 0x6c, 0xf7, 0xb4, 0xb7, 0xce, 0xc5, 0x86, 0x7a, 0x5c, 0x7a, 0x74, 0xd7, 0xe4, 0x90, 0xeb, 
        0x38, 0x8b, 0x8d, 0x5c, 0xf1, 0x89, 0xaa, 0x61, 0xc5, 0xd4, 0x19, 0x68, 0xf5, 0x67, 0x64, 0xb8, 
        0x29, 0xfe, 0xe4, 0x67, 0x87, 0x18, 0xcb, 0x8c, 0x00, 0x69, 0x0c, 0xc0, 0x96, 0xfe, 0x13, 0xdd, 
        0x1e, 0x94, 0xc6, 0xf3, 0xa8, 0x96, 0x72, 0x33, 0x47, 0x66, 0xd5, 0xf4, 0xd8, 0x79, 0xbe, 0x9f, 
        0xb0, 0x19, 0xb7, 0xe1, 0x4d, 0x37, 0xe3, 0xf7, 0x57, 0xe5, 0x70, 0x0d, 0xe2, 0xbe, 0xeb, 0xb5, 
        0xe4, 0x87, 0x7b, 0xc6, 0x49, 0x7d, 0x52, 0x00, 0x65, 0x21, 0x5d, 0x51, 0x57, 0x9c, 0xc1, 0x44
      };
    QByteArray half = QByteArray::fromRawData(halfRaw, sizeof(halfRaw));
    static const char fullRaw[] = {
        0x88, 0xe2, 0x56, 0x2f, 0x89, 0xab, 0x19, 0x56, 0x8b, 0x85, 0x2e, 0x8e, 0xd6, 0xae, 0x38, 0xf2, 
        0x37, 0x0f, 0xfd, 0xf9, 0xbc, 0xfc, 0x64, 0x0d, 0x68, 0x6b, 0xf6, 0xb6, 0x8f, 0x1e, 0xc1, 0x59, 
        0x91, 0x4e, 0xbf, 0x42, 0x22, 0x23, 0x9c, 0x16, 0xd9, 0xdc, 0xda, 0x3a, 0x01, 0x85, 0x3d, 0x44, 
        0x90, 0x18, 0x0b, 0xf9, 0x2d, 0x22, 0x0a, 0xf6, 0xe4, 0xa8, 0x8b, 0x58, 0x86, 0xac, 0xc7, 0xef, 
        0x52, 0xa0, 0x38, 0x7a, 0x13, 0xcc, 0x02, 0x37, 0x0f, 0x0c, 0xbf, 0xc3, 0xcc, 0xad, 0x33, 0xd3, 
        0xee, 0x3b, 0xad, 0xc2, 0x86, 0x40, 0x35, 0x14, 0x76, 0xf9, 0x92, 0x7c, 0x65, 0x1e, 0xcc, 0x24, 
        0x42, 0x07, 0x8b, 0x2a, 0x6c, 0xe7, 0x38, 0x98, 0x89, 0x25, 0xd4, 0x2b, 0x08, 0x10, 0x3d, 0x0b, 
        0x3c, 0xdb, 0x11, 0xda, 0x3b, 0xc4, 0x00, 0x56, 0xb7, 0xeb, 0x6a, 0xb4, 0x55, 0x5f, 0xb3, 0x3e, 
        0x48, 0xf6, 0x0b, 0xac, 0x2f, 0x27, 0xc7, 0xc1, 0xd5, 0x44, 0x53, 0x58, 0x17, 0x21, 0x77, 0x4d, 
        0x13, 0x4d, 0x7a, 0x0a, 0xd0, 0x35, 0x5c, 0x81, 0x42, 0x36, 0x5a, 0x76, 0x41, 0x64, 0x81, 0x9f, 
        0x03, 0x72, 0x17, 0xef, 0x23, 0x28, 0x30, 0xfc, 0x59, 0xb1, 0x48, 0xa1, 0x21, 0x93, 0xf8, 0x0b, 
        0x7f, 0xbf, 0x61, 0xdb, 0x76, 0xb7, 0x65, 0xdb, 0xa6, 0xed, 0x2f, 0x25, 0xa9, 0xbb, 0x49, 0xc7, 
        0x3c, 0x06, 0x69, 0x5e, 0xdd, 0xfa, 0x37, 0xcd, 0x66, 0x90, 0x11, 0x18, 0x6c, 0xfe, 0x8b, 0x6c, 
        0xb6, 0x38, 0xa5, 0xa3, 0xc7, 0x2a, 0x88, 0x86, 0xb8, 0xa4, 0x6d, 0xb9, 0x5f, 0xb4, 0xef, 0x86, 
        0x6e, 0x9d, 0x07, 0xd0, 0x13, 0x06, 0x19, 0xdb, 0xbe, 0x72, 0x79, 0xac, 0xb7, 0xe9, 0xc1, 0x4a, 
        0x20, 0x0a, 0x37, 0x75, 0x95, 0xbe, 0x4a, 0x71, 0x5c, 0xbc, 0xda, 0xa8, 0xc2, 0x2a, 0x8b, 0x96, 
        0x61, 0x55, 0x20, 0xda, 0x52, 0x74, 0x5e, 0x76, 0xc9, 0xfb, 0xaf, 0x69, 0xfb, 0xe0, 0x2e, 0x4c, 
        0xcf, 0xaa, 0xf6, 0xff, 0x6f, 0x90, 0x13, 0x2f, 0xa5, 0x6c, 0x4c, 0xee, 0x35, 0x23, 0xe9, 0x63, 
        0xfa, 0x31, 0xc1, 0x8f, 0x40, 0x7b, 0x8b, 0xff, 0x9c, 0x6f, 0x30, 0xfd, 0x63, 0xc8, 0x16, 0x5f, 
        0xf3, 0x6a, 0xb8, 0xde, 0xdc, 0x2c, 0xb0, 0xcc, 0x1b, 0xf9, 0xfd, 0x6b, 0xde, 0xa0, 0x75, 0x81, 
        0x57, 0x59, 0xcf, 0x31, 0x85, 0xfd, 0x80, 0x91, 0x3d, 0x6e, 0xc1, 0x4b, 0xd2, 0x64, 0x7a, 0x17, 
        0x39, 0xdb, 0xf7, 0xf2, 0x2c, 0xeb, 0x74, 0xdf, 0xcf, 0xfc, 0x48, 0x7f, 0xe8, 0x1c, 0x96, 0xa3, 
        0x5a, 0xff, 0xc8, 0xdc, 0x72, 0x84, 0x3d, 0x87, 0x60, 0x41, 0xb6, 0xeb, 0x4e, 0xae, 0xff, 0xc5, 
        0x4c, 0x92, 0x37, 0xec, 0x8e, 0x38, 0x4f, 0x1f, 0x44, 0x91, 0x0a, 0xf4, 0x72, 0xf3, 0x66, 0x8d, 
        0x2d, 0x5a, 0xb8, 0x39, 0x55, 0xde, 0x6b, 0xc4, 0xb6, 0xe7, 0xda, 0x43, 0x94, 0x94, 0x18, 0xcf, 
        0xd1, 0xb4, 0x92, 0x1e, 0x3a, 0xc7, 0x03, 0xd1, 0xa9, 0xbb, 0x23, 0x5a, 0xb4, 0xac, 0x4c, 0x66, 
        0x73, 0x14, 0xfc, 0xcf, 0x24, 0x78, 0xbf, 0xd0, 0x4e, 0xd2, 0xce, 0xd6, 0x2c, 0xfc, 0x8a, 0xb6, 
        0x8d, 0x9f, 0xe4, 0x8b, 0x8b, 0xca, 0xa2, 0x9d, 0xac, 0x7f, 0x01, 0x42, 0x10, 0x53, 0x52, 0x65, 
        0x06, 0xf8, 0x3c, 0x2c, 0x2b, 0x17, 0xeb, 0xb1, 0x2c, 0x1d, 0x91, 0x71, 0xba, 0x75, 0x71, 0xd3, 
        0x74, 0xa5, 0xa7, 0xf9, 0x0a, 0x84, 0x3d, 0x78, 0x43, 0x58, 0x5a, 0x87, 0xe8, 0x0e, 0xdf, 0x76, 
        0x7b, 0xc3, 0xbf, 0x85, 0x56, 0xf4, 0x7f, 0x5e, 0xf8, 0x0f, 0x10, 0xa5, 0x80, 0x55, 0x2b, 0x82, 
        0x85, 0xff, 0x19, 0x2e, 0x08, 0xbc, 0xb5, 0x29, 0x9e, 0xb3, 0xb7, 0xb6, 0x45, 0xce, 0xd8, 0x35, 
    };
    QByteArray full = QByteArray::fromRawData(fullRaw, sizeof(fullRaw));

    // empty data
    idnpacketizer.setupIdnDmx(data, 4, 0, empty, 1, 0, true);
    QCOMPARE(data.size(), 24);
    QCOMPARE(data.mid(0, 4).data(), idnhello.data());
    QCOMPARE(data.mid(4, 4).data(), "\x00\x14\xc0\x18");
    QCOMPARE(data.mid(12, 4).data(), idnSimpleConfig.data());
    QCOMPARE(data.mid(16).data(), simpleServiceConfig.data());

    // packet with 256 DMX channels
    idnpacketizer.setupIdnDmx(data, 4, 0, half, 1, 0, true);
    QCOMPARE(data.size(), 280);
    QCOMPARE(data.mid(0, 4).data(), idnhello.data());
    QCOMPARE(data.mid(4, 4).data(), "\x01\x14\xc0\x18");
    QCOMPARE(data.mid(12, 4).data(), idnSimpleConfig.data());
    QCOMPARE(data.mid(16, 8).data(), simpleServiceConfig.data());
    QCOMPARE(data.mid(24).data(), half.data());

    // packet with 512 DMX channels
    idnpacketizer.setupIdnDmx(data, 4, 0, full, 1, 0, true);
    QCOMPARE(data.size(), 536);
    QCOMPARE(data.mid(0, 4).data(), idnhello.data());
    QCOMPARE(data.mid(4, 4).data(), "\x02\x14\xc0\x18");
    QCOMPARE(data.mid(12, 16).data(), idnSimpleConfig.data());
    QCOMPARE(data.mid(16, 4).data(), simpleServiceConfig.data());
    QCOMPARE(data.mid(24).data(), full.data());

    //empty data in optimized mode
    const QList<QPair<int, int> > emptyRange; 
    idnpacketizer.setupIdnDmx(data, 5, 0, empty, emptyRange, 0, true);
    QCOMPARE(data.size(), 20);
    QCOMPARE(data.mid(0, 4).data(), idnhello.data());
    QCOMPARE(data.mid(4, 8).data(), "\x00\x10\xc0\x18");
    QCOMPARE(data.mid(12, 4).data(), idnNoServiceModeConfig.data());

    //half data in optimized mode
    QList<QPair<int, int> > simpleHalfRange;
    simpleHalfRange.append(qMakePair(1,256));
    idnpacketizer.setupIdnDmx(data, 5, 0, half, simpleHalfRange, 0, true);
    QCOMPARE(data.size(), 280);
    QCOMPARE(data.mid(0, 4).data(), idnhello.data());
    QCOMPARE(data.mid(4, 4).data(), QByteArray("\x01\x14\xc0\x18").data());
    QCOMPARE(data.mid(12, 4).data(), idnSimpleConfig.data());
    QCOMPARE(data.mid(16, 4).data(), halfServiceModeConfig.data());
    QCOMPARE(data.mid(24).data(), half.data());
    
    // //full data in optimized mode
    QList<QPair<int, int> > simpleFullRange;
    simpleFullRange.append(qMakePair(1,512));
    idnpacketizer.setupIdnDmx(data, 5, 0, full, simpleFullRange, 0, true);
    QCOMPARE(data.size(), 536);
    QCOMPARE(data.mid(0, 4).data(), idnhello.data());
    QCOMPARE(data.mid(4, 4).data(), "\x02\x14\xc0\x18");
    QCOMPARE(data.mid(12, 4).data(), idnSimpleConfig.data());
    QCOMPARE(data.mid(16, 4).data(), fullServiceModeConfig.data());
    QCOMPARE(data.mid(24), full);
}

QTEST_MAIN(Idn_Test)