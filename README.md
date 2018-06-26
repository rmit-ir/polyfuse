# Polyfuse

A tool for performing popular fusion techniques on TREC run files.

The following fusion methods are supported:

* Borda count
* CombANZ
* CombMAX
* CombMED
* CombMIN
* CombMNZ
* CombSUM
* Inverse square rank
* Logarithmic inverse square rank
* Rank-biased centroids
* Reciprocal rank fusion

## Usage

Fuse three runs using CombSUM to depth 100:

`polyfuse combsum -d100 a.run b.run c.run > combsum.run`

To see all fusion commands and options run `polyfuse -h`.
