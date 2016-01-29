# Copyright (C)  2012-2016   Mark Seligman
##
## This file is part of ArboristBridgeR.
##
## ArboristBridgeR is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 2 of the License, or
## (at your option) any later version.
##
## ArboristBridgeR is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with ArboristBridgeR.  If not, see <http://www.gnu.org/licenses/>.

# 'y' should be defined on entry.  Other entry points to be used for different behaviours.
#
"Rborist.default" <- function(x, y, nTree=500, withRepl = TRUE,
                ctgCensus = "votes",
                classWeight = NULL,
                nSamp = 0,
                minInfo = 0.01,
                minNode = ifelse(is.factor(y), 2, 5),
                nLevel = 0,
                predFixed = 0,
                predProb = 0.0,
                predWeight = NULL, 
                qVec = NULL,
                quantiles = !is.null(qVec),
                qBin = 5000,
                rowWeight = NULL,
                treeBlock = 1,
                pvtBlock = 8, pvtNoValidate = FALSE, ...) {

  # Argument checking:
  if (inherits(x, "PreTrain")) {
    preTrain <- x 
  }
  else {
    preTrain <- PreTrain(x)
  }
  predBlock <- preTrain$predBlock
  nPred <- predBlock$nPredNum + predBlock$nPredFac
  nRow <- predBlock$nRow

  if (nSamp == 0) {
    nSamp <- ifelse(withRepl, nRow, round((1-exp(-1)) * nRow))
  }
  if (predFixed == 0) {
    predFixed <- ifelse(nPred >= 16, 0, ifelse(!is.factor(y), max(floor(nPred/3), 1), floor(sqrt(nPred))))
  }
  if (predProb == 0.0) {
    predProb <- ifelse(predFixed != 0, 0.0, ifelse(!is.factor(y), 0.4, ceiling(sqrt(nPred))/nPred))
  }
  if (is.null(predWeight)) {
    predWeight <- rep(1.0, nPred)
  }
  
  if (any(is.na(y)))
    stop("NA not supported in response")
  if (!is.numeric(y) && !is.factor(y))
    stop("Exping numeric or factor response")

  # Class weights
  if (!is.null(classWeight)) {
    if (is.numeric(classWeight)) {
    }
    else if (classWeight == "auto") {
    }
    else {
      stop("Unrecognized class weights")
    }
  }  
  
  # Height constraints
  if (minNode < 1)
    stop("Minimum node size must be positive")
  else if (minNode > nSamp)
    stop("Minimum splitting width exceeds sample count")

  # Predictor weight constraints
  if (length(predWeight) != nPred)
    stop("Length of predictor weight does not equal number of columns")
  if (any(predWeight < 0))
    stop("Negative predictor weights")
  if (all(predWeight == 0))
    stop("All predictor weights zero")
  # TODO:  Ensure all pred weights are numeric
  
  # No holes allowed in response
  if (is.factor(y) && any(table(y) == 0))
    stop("Empty classes not supported in response.")

  # Sample weight constraints.
  if (!is.null(rowWeight)) {
    if (length(rowWeight) != nRow)
      stop("Sample weight length must match row count")
    
    if (any(rowWeight < 0))
      stop("Negative weights not permitted")
  }
  else {
    rowWeight = rep(1.0, nRow)
  }
  
  # Quantile constraints:  regression only
  if (quantiles && is.factor(y))
    stop("Quantiles supported for regression case only")

  if (!is.null(qVec)) {
    if (any(qVec > 1) || any(qVec < 0))
      stop("Quantile range must be within [0,1]")
    if (any(diff(qVec) <= 0))
      stop("Quantile range must be increasing")
  }

  if (pvtNoValidate)
    stop("Non-validating version NYI")
  
  # Normalizes vector of pointwise predictor probabilites.
  #if (predProb != 0 && predFixed != 0)
   # stop("Only one of 'predProb' and 'predFixed' may be specified")
  if (length(predProb) > 1)
    stop("'predProb' must have a scalar value")
  if (predProb < 0 || predProb > 1.0)
    stop("'predProb' value must lie in [0,1]")
  if (predFixed < 0 || predFixed > nPred)
    stop("'predFixed' must be positive integer <= predictor count")

  meanWeight <- ifelse(predProb == 0.0, 1.0, predProb)
  probVec <- predWeight * (nPred * meanWeight) / sum(predWeight)

  if (is.factor(y)) {
    train <- .Call("RcppTrainCtg", preTrain$predBlock, preTrain$rowRank, y, nTree, nSamp, rowWeight, withRepl, treeBlock, minNode, minInfo, nLevel, predFixed, probVec)
  }
  else {
    train <- .Call("RcppTrainReg", preTrain$predBlock, preTrain$rowRank, y, nTree, nSamp, rowWeight, withRepl, treeBlock, minNode, minInfo, nLevel, predFixed, probVec)
  }

  predInfo <- train[["predInfo"]]
  names(predInfo) <- preTrain$predBlock$colnames
  training = list(
    info = predInfo,
    bag = train[["bag"]]
  )

  if (!pvtNoValidate) {
    if (is.factor(y)) {
      if (ctgCensus == "votes") {
        validation <- .Call("RcppValidateVotes", preTrain$predBlock, train$forest, train$leaf, y, training[["bag"]]);
      }
      else if (ctgCensus == "prob") {
        validation <- .Call("RcppValidateProb", preTrain$predBlock, train$forest, train$leaf, y, training[["bag"]]);
      }
      else {
        stop(paste("Unrecognized ctgCensus type:  ", ctgCensus))
      }
    }
    else {
      if (quantiles) {
        if (is.null(qVec)) {
          qVec <- DefaultQuantVec()
        }
        validation <- .Call("RcppValidateQuant", preTrain$predBlock, train$forest, train$leaf, qVec, qBin, y, training[["bag"]])
      }
      else {
        validation <- .Call("RcppValidateReg", preTrain$predBlock, train$forest, train$leaf, y, training[["bag"]])
      }
    }
  }
  else {
    validation <- NULL
  }

  arbOut <- list(
    forest = train$forest,
    leaf = train$leaf,
    training = training,
    validation = validation
  )
  class(arbOut) <- "Rborist"

  arbOut
}


PreTrain <- function(x) {
  # Argument checking:
  if (any(is.na(x)))
    stop("NA not supported in design matrix")

  predBlock <- PredBlock(x)
  rowRank <- .Call("RcppRowRank", predBlock)

  preTrain <- list(
    predBlock = predBlock,
    rowRank = rowRank
  )
  class(preTrain) <- "PreTrain"

  preTrain
}



# Groups predictors into like-typed blocks and creates zero-based type
# summaries.
#
PredBlock <- function(x) {
  # For now, only numeric and factor types supported.
  #
  if (is.data.frame(x)) { # As with "randomForest" package
    facCard <- as.integer(sapply(x, function(col) ifelse(is.factor(col) && !is.ordered(col), length(levels(col)), 0)))
    numCols <- as.integer(sapply(x, function(col) ifelse(is.numeric(col), 1, 0)))
    nPredFac <- length(which(facCard > 0))
    nPredNum <- length(which(numCols > 0))
    if (nPredFac + nPredNum != ncol(x)) {
      stop("Frame column with unsupported data type")
    }
    return(.Call("RcppPredBlockFrame", x, nPredNum, nPredFac, facCard))
  }
  else if (is.integer(x)) {
    return(.Call("RcppPredBlockNum", data.matrix(x)))
  }
  else if (is.numeric(x)) {
    return(.Call("RcppPredBlockNum", x))
  }
  else if (is.character(x)) {
    stop("Character data not yet supported")
  }
  else {
    stop("Unsupported data format")
  }
}


# Uses quartiles by default.
#
DefaultQuantVec <- function() {
  seq(0.25, 1.0, by = 0.25)
}