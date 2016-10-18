// This file is part of ArboristCore.

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
   @file rowrank.cc

   @brief Methods for predictor-specific training.

   @author Mark Seligman
 */

#include "rowrank.h"
#include "predblock.h"

#include <algorithm>

// Testing only:
//#include <iostream>
//using namespace std;

// Observations are blocked according to type.  Blocks written in separate
// calls from front-end interface.


/**
   @brief Numeric predictor presort to parallel output vectors.

   @param feNum is a block of numeric predictor values.

   @param nPredNum is the number of numeric predictors.

   @param nRow is the number of observation rows. 

   @param rank outputs the tie-classed predictor ranks.


   @output void, with output vector parameters.
 */
void RowRank::PreSortNum(const double _feNum[], unsigned int _nPredNum, unsigned int _nRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rlOut, std::vector<unsigned int> &numOffOut, std::vector<double> &numOut) {
  unsigned int outColOff = 0;
  for (unsigned int numIdx = 0; numIdx < _nPredNum; numIdx++) {
    numOffOut[numIdx] = outColOff;
    outColOff += NumSortRaw(&_feNum[numIdx * _nRow], _nRow, rowOut, rankOut, rlOut, numOut);
  }
}


void RowRank::PreSortNumRLE(const std::vector<double> &valNum, const std::vector<unsigned int> &rowStart, const std::vector<unsigned int> &runLength, unsigned int _nPredNum, unsigned int _nRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rlOut, std::vector<unsigned int> &numOffOut, std::vector<double> &numOut) {
  unsigned int colOff = 0;
  for (unsigned int numIdx = 0; numIdx < _nPredNum; numIdx++) {
    numOffOut[numIdx] = colOff;
    unsigned int idxCol = NumSortRLE(&valNum[colOff], _nRow, &rowStart[colOff], &runLength[colOff], rowOut, rankOut, rlOut, numOut);
    colOff += idxCol;
  }
}


/**
   @brief Sorts a column of numerical predictor values compressed with
   run-length encoding.

   @return Count of input vector elements read for the column.
 */
unsigned int RowRank::NumSortRLE(const double colNum[], unsigned int _nRow, const unsigned int rowStart[], const unsigned int runLength[], std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rleOut, std::vector <double> &numOut) {
  std::vector<RLENum> rleNum;
  for (unsigned int rleIdx = 0, rowTot = 0; rowTot < _nRow; rowTot += runLength[rleIdx++]) {
    rleNum.push_back(std::make_tuple(colNum[rleIdx], rowStart[rleIdx], runLength[rleIdx]));
  }

  std::sort(rleNum.begin(), rleNum.end()); // runlengths silent, as rows unique.
  RankNum(rleNum, rowOut, rankOut, rleOut, numOut);

  return rleNum.size();
}


/**
   @brief

   @return void.
 */
unsigned int RowRank::NumSortRaw(const double colNum[], unsigned int _nRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rleOut, std::vector<double> &numOut) {
  std::vector<ValRowD> valRow(_nRow);
  for (unsigned int row = 0; row < _nRow; row++) {
    valRow[row] = std::make_pair(colNum[row], row);
  }

  std::sort(valRow.begin(), valRow.end());  // Stable sort.
  return RankNum(valRow, rowOut, rankOut, rleOut, numOut);
}


/**
   @brief Stores ordered predictor column, entering uncompressed.

   @param numOut outputs the rank-ordered predictor values.

   @return void.
 */
unsigned int RowRank::RankNum(const std::vector<ValRowD> &valRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rleOut, std::vector<double> &numOut) {
  unsigned int rk = 0;
  unsigned int topNum = numOut.size();
  rleOut.push_back(1);
  rowOut.push_back(valRow[0].second);
  numOut.push_back(valRow[0].first);
  rankOut.push_back(rk);
  for (unsigned int idx = 1; idx < valRow.size(); idx++) {
    double valThis = valRow[idx].first;
    unsigned int rowThis = valRow[idx].second;

    if (valThis == numOut.back() && rowThis == rowOut.back() + rleOut.back()) {
      rleOut.back()++;
    }
    else { // New RLE, row and rank entries regardless whether tied.
      rleOut.push_back(1);
      rowOut.push_back(rowThis);
      rankOut.push_back(rk);
      if (valThis != numOut.back()) { // New rank, num entry iff not tied.
	numOut.push_back(valThis);
	rk++;
      }
    }
  }

  return numOut.size() - topNum;
}


/**
   @brief Stores ordered predictor column compresed by external RLE.

   @return void.
 */
void RowRank::RankNum(const std::vector<RLENum> &rleNum, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rleOut, std::vector<double> &numOut) {
  RLENum elt = rleNum[0];
  unsigned int rk = 0;
  numOut.push_back(std::get<0>(elt));
  rowOut.push_back(std::get<1>(elt));
  rleOut.push_back(std::get<2>(elt));
  rankOut.push_back(rk);
  for (unsigned int idx = 1; idx < rleNum.size(); idx++) {
    elt = rleNum[idx];
    double valThis = std::get<0>(elt);
    unsigned int rowThis = std::get<1>(elt);
    unsigned int runCount = std::get<2>(elt);
    if (valThis == numOut.back() && rowThis == rowOut.back() + rleOut.back()) {
      rleOut.back() += runCount;
    }
    else { // New RLE, rank entries regardless whether tied.
      rleOut.push_back(runCount);
      rowOut.push_back(rowThis);
      rankOut.push_back(rk);
      if (valThis != numOut.back()) {
	numOut.push_back(valThis);
	rk++;
      }
    }
  }
}


/**
   @brief Factor predictor presort to parallel output vectors.

   @param feFac is a block of factor predictor values.

   @param _facStart is the starting output position for factors.

   @param nPredFac is the number of factor predictors.

   @param nRow is the number of observation rows. 

   @param rowOrd outputs the (unstably) sorted row indices.

   @param rank Outputs the tie-classed predictor ranks.

   @output void, with output vector parameters.
 */
void RowRank::PreSortFac(const unsigned int _feFac[], unsigned int _nPredFac, unsigned int _nRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &runLength) {
  // Builds the ranked factor block.  Assumes 0-justification has been 
  // performed by bridge.
  //
  for (unsigned int facIdx = 0; facIdx < _nPredFac; facIdx++) {
    FacSort(&_feFac[facIdx * _nRow], _nRow, rowOut, rankOut, runLength);
  }
}


/**
   @brief Sorts factors and stores as rank-ordered run-length encoding.

   @return void.
 */
void RowRank::FacSort(const unsigned int predCol[], unsigned int _nRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rleOut) {
  std::vector<ValRowI> valRow(_nRow);
  for (unsigned int row = 0; row < _nRow; row++) {
    valRow[row] = std::make_pair(predCol[row], row);
  }
  std::sort(valRow.begin(), valRow.end()); // Stable sort.
  RankFac(valRow, rowOut, rankOut, rleOut);
}


/**
   @brief Builds rank-ordered run-length encoding to hold factor values.

   Final "rank" values are the internal factor codes and may contain
   gaps.  A dense numbering scheme would entail backmapping at LH bit
   assignment following splitting (q.v.):  prediction and training
   must map to the same factor levels.

   @return void.
*/ 
void RowRank::RankFac(const std::vector<ValRowI> &valRow, std::vector<unsigned int> &rowOut, std::vector<unsigned int> &rankOut, std::vector<unsigned int> &rleOut) {
  unsigned int rankPrev = valRow[0].first;
  unsigned int rowPrev = valRow[0].second;
  rleOut.push_back(1);
  rankOut.push_back(rankPrev);
  rowOut.push_back(rowPrev);
  for (unsigned int row = 1; row < valRow.size(); row++) {
    unsigned int rankThis = valRow[row].first;
    unsigned int rowThis = valRow[row].second;

    if (rankThis == rankPrev && rowThis == (rowPrev + 1)) {
      rleOut.back() ++;
    }
    else {
      rleOut.push_back(1);
      rankOut.push_back(rankThis);
      rowOut.push_back(rowThis);
    }
    rankPrev = rankThis;
    rowPrev = rowThis;
  }
}



/**
   @brief Constructor for row, rank passed from front end as parallel arrays.

   @param feRow is the vector of rows allocated by the front end.

   @param feRank is the vector of ranks allocated by the front end.

 */
RowRank::RowRank(const PMTrain *pmTrain, const std::vector<unsigned int> &feRow, const std::vector<unsigned int> &feRank, const std::vector<unsigned int> &_numOffset, const std::vector<double> &_numVal, const std::vector<unsigned int> &feRunLength) : nRow(pmTrain->NRow()), nPred(pmTrain->NPred()), noRank(std::max(nRow, pmTrain->CardMax())), numOffset(_numOffset), numVal(_numVal), nonCompact(0), accumCompact(0) {
  // Default initialization to uncompressed values.
  rrCount.reserve(nPred);
  rrStart.reserve(nPred);
  safeOffset.reserve(nPred);
  denseRank.reserve(nPred);
  for (unsigned int predIdx = 0; predIdx < nPred; predIdx++) {
    denseRank.push_back(noRank);
    rrCount.push_back(0);
    rrStart.push_back(0);
    safeOffset.push_back(0);
  }

  unsigned int blockTot = DenseBlock(feRank, feRunLength);
  rrNode = new RRNode[blockTot];

  Decompress(feRow, feRank, feRunLength);
}


/**
   @brief Counts the number of rows to be decompressed and sets dense ranks.

   @param rle records the run lengths of the remaining entries.

   @param nonCmprTot is the total number of noncompressed rows.

   @return total number of rows to be decompressed.
 */
unsigned int RowRank::DenseBlock(const std::vector<unsigned int> &feRank, const std::vector<unsigned int> &rle) {
  unsigned int rleIdx = 0;
  for (unsigned int predIdx = 0; predIdx < nPred; predIdx++) {
    unsigned int denseMax = 0; // Running maximum of run counts.
    unsigned int argMax = noRank;
    unsigned int runCount = 0; // Runs across adjacent rle entries.
    unsigned int rankPrev = noRank;
    
    for (unsigned int rowTot = rle[rleIdx]; rowTot <= nRow; rowTot += rle[rleIdx]) {
      unsigned int runLength = rle[rleIdx];
      unsigned int rankThis = feRank[rleIdx];
      if (rankThis == rankPrev) {
	runCount += runLength;
      }
      else {
	runCount = runLength;
	rankPrev = rankThis;
      }
      if (runCount > denseMax) {
	denseMax = runCount;
	argMax = rankThis;
      }
      rleIdx++;
      if (rleIdx == rle.size())
	break;
    }
    // Post condition:  rowTot == nRow.

    unsigned int rowCount;
    if (denseMax > plurality * nRow) {
      denseRank[predIdx] = argMax;
      safeOffset[predIdx] = accumCompact; // Accumulated offset:  dense.
      rowCount = nRow - denseMax;
      accumCompact += rowCount;
    }
    else {
      denseRank[predIdx] = noRank;
      safeOffset[predIdx] = nonCompact++; // Index:  non-dense storage.
      rowCount = nRow;
    }
    rrCount[predIdx] = rowCount;
  }

  // Assigns rrNode[] offsets ut noncompressed predictors stored first,
  // as with staging offsets.
  //
  unsigned int blockTot = 0;
  for (unsigned int predIdx = 0; predIdx < nPred; predIdx++) {
    unsigned int offSafe = safeOffset[predIdx];
    if (denseRank[predIdx] != noRank) {
      rrStart[predIdx] = nonCompact * nRow + offSafe;
    }
    else {
      rrStart[predIdx] = offSafe * nRow;
    }
    blockTot += rrCount[predIdx];
  }
  
  return blockTot;
}


/**
   @brief Decompresses a block of predictors having compressed encoding.

   @param rle records the run lengths of the remaining entries.

   @param predStart is the first predictor in the block.

   @return void.
 */
void RowRank::Decompress(const std::vector<unsigned int> &feRow, const std::vector<unsigned int> &feRank, const std::vector<unsigned int> &rle) {
  unsigned int rleIdx = 0;
  for (unsigned int predIdx = 0; predIdx < nPred; predIdx++) {
    unsigned int outIdx = rrStart[predIdx];
    for (unsigned int rowTot = rle[rleIdx]; rowTot <= nRow; rowTot += rle[rleIdx]) {
      unsigned int runLength = rle[rleIdx];
      unsigned int rank = feRank[rleIdx];
      if (rank != denseRank[predIdx]) { // Omits dense ranks.
	for (unsigned int i = 0; i < runLength; i++) { // Expands runs.
	  RRNode rr;
	  rr.Init(feRow[rleIdx] + i, rank);
	  rrNode[outIdx++] = rr;
	}
      }
      rleIdx++;
      if (rleIdx == rle.size())
	break;
    }
    //    if (outIdx - rrStart[predIdx] != rrCount[predIdx])
    //cout << "Dense count mismatch" << endl;
  }
}


/**
   @brief Deallocates and resets.

   @return void.
 */
RowRank::~RowRank() {
  delete [] rrNode;
}
