# Layout
Layout simplifies overlaps graph and outputs `contigs.afg` file.

## Usage
```
usage: ./bin/layout --reads=string --overlaps=string [options] ...
options:
  -r, --reads              reads file (string)
  -s, --reads_format       reads format; supported: fasta, afg (string [=fasta])
  -a, --read_id_offset     reads id offset (first read id) (int [=0])
  -x, --overlaps           overlaps file (string)
  -v, --verbose            verbose output (bool [=0])
  -f, --overlaps_format    overlaps file format; supported: afg, mhap (string [=afg])
  -?, --help               print this message
```

## Steps
- filtering containment edges
- filtering transitive edges
- trimming
- bubble popping
