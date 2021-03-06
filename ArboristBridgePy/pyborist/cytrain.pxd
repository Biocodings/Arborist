# distutils: language = c++

from libcpp cimport bool
from libcpp.vector cimport vector

from .cyforest cimport ForestNode
from .cyleaf cimport LeafNode
from .cyleaf cimport BagRow



cdef extern from 'train.h':
    cdef void Train_Init 'Train::Init'(double *_feNum,
        int _facCard[],
        int _cardMax,
        int _nPredNum,
        int _nPredFac,
        int _nRow,
        int _nTree,
        int _nSamp,
        double _feSampleWeight[],
        bool withRepl,
        int _trainBlock,
        int _minNode,
        double _minRatio,
        int _totLevels,
        int _ctgWidth,
        int _predFixed,
        double _predProb[],
        double _regMono[])

    cdef void Train_Regression 'Train::Regression'(int _feRow[],
        int _feRank[],
        int _feInvNum[],
        const vector[double] &_y,
        const vector[unsigned int] &_row2Rank,
        vector[unsigned int] &_origin,
        vector[unsigned int] &_facOrigin,
        double _predInfo[],
        vector[ForestNode] &_forestNode,
        vector[unsigned int] &_facSplit,
        vector[unsigned int] &_leafOrigin,
        vector[LeafNode] &_leafNode,
        vector[BagRow] &_bagRow,
        vector[unsigned int] &_rank)

    cdef void Train_Classification 'Train::Classification'(int _feRow[],
        int _feRank[],
        int _feInvNum[],
        const vector[unsigned int]  &_yCtg,
        int _ctgWidth,
        const vector[double] &_yProxy,
        vector[unsigned int] &_origin,
        vector[unsigned int] &_facOrigin,
        double _predInfo[],
        vector[ForestNode] &_forestNode,
        vector[unsigned int] &_facSplit,
        vector[unsigned int] &_leafOrigin,
        vector[LeafNode] &_leafNode,
        vector[BagRow] &_bagRow,
        vector[double] &_weight)
