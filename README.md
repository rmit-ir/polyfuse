# Polyfuse

A tool for performing popular fusion techniques on TREC run files.

The following fusion methods are supported:

* Borda count
* CombANZ
* CombMNZ
* CombSUM
* Inverse square rank
* Logarithmic inverse square rank
* Rank-biased centroids
* Reciprocal rank fusion

## Changelog
29 May 2018 -- An off-by-one error has been resolved where the first document for every topic was
missing in the TREC output. An empirical test on CombSUM first went from an AP of 0.314 to 0.324 after
the fixes, on the NYT collection.

Additionally, the rank number is only added to the output of RBC, to keep the document score 
distributions consistent with other fusion frameworks.

## Installation

Run `make`. Requires gcc.

This will generate the `polyfuse` binary.

## Example usage

For regular system fusion, here is an example of using CombSUM to depth 100 using minmax scaling:

`./polyfuse combsum -n minmax -d 100 run1 run2`


