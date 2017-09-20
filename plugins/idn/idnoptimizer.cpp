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
#include "idnoptimizer.h"

#include <QTextStream>

QTextStream out33(stdout);

IdnOptimizer::IdnOptimizer(){
    oldData.fill(0x00, 512);
}

IdnOptimizer::~IdnOptimizer(){}


QList<QPair<int, int> > IdnOptimizer::optimize(const QByteArray& data, const bool checkNullValues){
    QByteArray newData = data;
    QByteArray fill;
    newData.append(fill.fill(0x00, 512), 512-data.length());
    QList<int> changedVal = changedValues(oldData, newData);
    changed = changed.unite(changedVal.toSet());

    oldData = newData;

    QMutableSetIterator<int> i(changed);
    while(i.hasNext()){
        int val = i.next();

        if(oldData.at(val) == 0x00){
            nullChannelChecker[val] = nullChannelChecker[val]+1;
        }

        if(checkNullValues && nullChannelChecker[val] > 5){
            changed.remove(val);
            nullChannelChecker[val] = 0;
        }
    }
    return getRanges(changed.toList());
}

QList<int> IdnOptimizer::changedValues(QByteArray oldData, QByteArray newData){
  QList<int> changedChannelBuffer;
  for(int i = 0; i < oldData.length(); i++){
      if(newData.at(i) == oldData.at(i)){
          continue;
        }else{
          changedChannelBuffer << i;
      }
     }
  return changedChannelBuffer;
}

QList<QPair<int, int> > IdnOptimizer::getRanges(QList<int> changed){
  qSort(changed.begin(), changed.end(), qGreater<int>());
  rangeList.clear();
  
  QPair<int, int> range;

  for(int i = 0; i < changed.length(); i++){
    if(i > 0 && i < changed.length()-1){
      if(changed[i] - changed[i+1] == 1 && changed[i-1]-changed[i] != 1){
        //start of contiguous channels
        range.second = changed[i];
        continue;
      }

      if(changed[i] - changed[i+1] != 1 && changed[i-1] - changed[i] == 1){
        //end of contiguous channels
        range.first = changed[i];
        rangeList.append(range);
        continue;
      }

      if(changed[i] - changed[i+1] != 1 && changed[i-1] - changed[i] != 1){
        //single channel
        range.first = changed[i];
        range.second = changed[i];
        rangeList.append(range);
        continue;
      }
    }else{
      if(i == 0 && changed.length() > 1 && changed[i]-changed[i+1] == 1){
        //start of contiguous row of channels
        range.second = changed[i];
        continue;
      }
      else if(i == changed.length()-1 && i > 0 && changed[i-1] - changed[i] == 1){
        //end of contiguous row of channels
        range.first = changed[i];
        //check if range-tuple has more than 256 values
        if(range.second-range.first+1 > 255){
            rangeList.append(qMakePair(range.first, range.first+254));
            if(range.second-range.first+1 > 510){
                rangeList.append(qMakePair(range.first+255, range.first+509));
                rangeList.append(qMakePair(range.first+510, range.second));
            }else{
                rangeList.append(qMakePair(range.first+255, range.second));
            }
        }else{
            rangeList.append(range);
        }
        continue;
      }else{
        //single channel
        range.first = changed[i];
        range.second = changed[i];
        rangeList.append(range);
        continue;
      }
    }
  }
  return rangeList;
}
