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

Fuse three runs using CombSUM with scaled normalization to depth 100:

```polyfuse combsum -d 100 -n minmax a.run b.run c.run > combsum.run```

To see all fusion commands and options run `polyfuse -h`.

To try all fusion methods run `tools/sweep_polyfuse.py a.run b.run c.run` and the output will be saved in `fusion_output/`.
