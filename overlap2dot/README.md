# overlap2dot
Converts overlaps to dot graph.

## Requirements
- make
- gcc
- graphviz (for plotting part)

## Installation

```
  make
```

## Usage

```
usage: ./bin/overlap2dot [options] <input_files...>
options:
  -f, --format    input file format; supported: afg, mhap (string [=afg])
  -?, --help      print this message
```

Example:

```
  ./bin/overlap2dot < overlaps.afg > graph.dot
```

Or even better, for direct plotting to a file

```
  ./bin/overlap2dot < overlaps.afg | neato -T png -o graph.png
```

## Edge type description
- full dot - overlap uses read's prefix
- empty dot - overlap uses read's suffix
- full square - overlap uses read's suffix and prefix.
