
## Arborist: Parallelized, Extensible Random Forests


[![License](https://img.shields.io/badge/core-MPL--2-brightgreen.svg)](https://www.mozilla.org/en-US/MPL/2.0/) 
[![R License](http://img.shields.io/badge/R_Bridge-GPL%20%28%3E=%202%29-brightgreen.svg?style=flat)](http://www.gnu.org/licenses/gpl-2.0.html)
[![Python License](http://img.shields.io/badge/Python__Bridge-MIT-brightgreen.svg?style=flat)](https://opensource.org/licenses/MIT
)
[![CRAN](http://www.r-pkg.org/badges/version/Rborist)](https://cran.rstudio.com/web/packages/Rborist/index.html)
[![Downloads](http://cranlogs.r-pkg.org/badges/Rborist?color=brightgreen)](http://www.r-pkg.org/pkg/Rborist)
[![PyPI version](https://badge.fury.io/py/pyborist.svg)](https://pypi.python.org/pypi/pyborist/) 
[![Travis-CI Build Status](https://travis-ci.org/suiji/Arborist.svg?branch=master)](https://travis-ci.org/suiji/Arborist)




The Arborist provides a fast, open-source implementation of Leo Brieman's Random Forest algorithm. The Arborist achieves its speed through efficient C++ code and parallel, distributed tree construction. 

Bindings are available for [R](https://cran.r-project.org/web/packages/Rborist/index.html) and **Python (coming soon)**


### R

The *Arborist* is available on CRAN as the [Rborist](https://cran.r-project.org/web/packages/Rborist/index.html) package. 

Installation of Release Version:

    > install.packages('Rborist')

Installation of Development Version:

    > ./ArboristBridgeR/Package/Rborist.CRAN.sh
    > R CMD INSTALL Rborist_*.*-*.tar.gz


#### Notes
- Version 0.1-6 now on CRAN.

### Python

Under development.  Contributors sought.

### Performance 

Performance metrics will be measured soon using [benchm-ml](https://github.com/szilard/benchm-ml). Partial results can be found [here](https://github.com/szilard/benchm-ml/tree/master/z-other-tools)

    
### References

- [Scalability Issues in Training Decision Trees (video)](https://www.youtube.com/watch?v=ol0SZ2Omq7w), Nimbix Developer Summit, 2017.
- [Controlling for Monotonicity in Random Forest Regressors (PDF)](http://past.rinfinance.com/agenda/2016/talk/MarkSeligman.pdf), R in Finance, May 2016.
- [ Accelerating the Random Forest algorithm for commodity parallel hardware (Video)](https://www.youtube.com/watch?v=dRZrYdhNUec), PyData, July, 2015.
- [The Arborist:  A High-Performance Random Forest (TM) Implementation](http://past.rinfinance.com/agenda/2015/talk/MarkSeligman.pdf), R in Finance, May 2015.
- [Training Random Forests on the GPU:  Tree Unrolling (PDF)](http://on-demand.gputechconf.com/gtc/2015/posters/GTC_2015_Machine_Learning___Deep_Learning_03_P5282_WEB.pdf), GTC, March, 2015.


### News/Changes
- New option 'autoCompress' to fine-tune sparse internal representation.
- New function 'Streamline' to clear unused fields of trained forest.
- Sparse R-style 'dcgMatrix' format now accepted, with "i/p" encoding.
- Autocompression employed on a per-predictor basis.
- Space-saving 'thinLeaves' option suppresses creation of summary data for narrow workflows.
- Research-inspired option 'splitQuantile' provides fine-tuning of numeric split positions.
- Improved scaling with row count.
Correctness errors are being addressed as they are received.

