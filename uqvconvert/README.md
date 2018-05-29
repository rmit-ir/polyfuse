# UQVConvert

This tool converts UQV TREC run files into their respective polyfuseable runs.

UQV trec run files take the form:

```
301-1 Q0 FBIS3-23986 1 -7.1317 indri
301-1 Q0 FBIS4-38364 2 -7.13401 indri
301-1 Q0 FBIS4-41991 3 -7.14264 indri
...
```

Invoke by using `./uqvpolyfuse.sh uqvrun1 uqvrun2 ...`

The fused run will be named `out.txt` in this folder.
