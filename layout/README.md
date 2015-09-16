# Layout
Layout creates rough layout from given reads and overlaps.

As a result of its' work following files will be created:
- **contigs.afg** - computed layout of genome
- **untigis.afg** - unitigs structure (list of reads and positions)
- **contigs_fast.fasta** - rough contigs made just by putting raw reads on computed positions
- **unitigs_fast.fasta** - rough unitigs made just by putting raw reads on computed positions
- **simplified.(*:overlaps_format*)** - overlaps that have not been filtered during graph simplification steps
- **nocont.(*:overlaps_format*)** - overlaps not filtered by first simplification step
- **nocont.notran.(*:overlaps_format*)** - overlaps not filtered by first two simplification steps
- **run_args.txt** - all used parameters, the very same file format is used for defining params (you can reuse the file)

## Steps
- graph simplification
  - filtering containment edges
  - filtering transitive edges
  - trimming
  - bubble popping
- extract unitigs
- extract cotigs (*experimental*)

## Usage
```
usage: ./bin/layout --reads=string --overlaps=string [options] ...
options:
  -r, --reads              reads file (string)
  -s, --reads_format       reads format; supported: fasta, fastq, afg (string [=fasta])
  -a, --reads_id_offset    reads id offset (first read id) (int [=0])
  -x, --overlaps           overlaps file (string)
  -f, --overlaps_format    overlaps file format; supported: afg, mhap (string [=afg])
  -b, --settings           settings file (string [=])
  -?, --help               print this message
```

## Example

with default settings
```
./bin/layout -r reads.fasta -f mhap -x overlaps.mhap
```

with settings file
```
./bin/layout -r reads.fasta -f mhap -x overlaps.mhap -b settings.txt
```

## Settings file
Let's look how one settings file could look like:
(btw all lines starting with # are comments)

```
# version: 9ab697a-dirty
#/Users/mario/ra/bin/layout --overlaps_format=afg -r reads-nmeth-all_2d.fasta -f mhap -x overlaps.mhap -b settings.txt
# filter reads parameters
READS_MIN_LEN: 3000

# filter overlaps parameters
OVERLAPS_MIN_QUALITY: 0.150000

# trimming parameters
READ_LEN_THRESHOLD: 100000
MAX_READS_IN_TIP: 2
MAX_DEPTH_WITHOUT_EXTRA_FORK: 5

# bubble popping parameters
MAX_NODES: 160
MAX_DISTANCE: 1600000
MAX_DIFFERENCE: 0.250000

# contig extraction parameters
MAX_BRANCHES: 18
MAX_START_NODES: 100
LENGTH_THRESHOLD: 0.050000
QUALITY_THRESHOLD: 0.200000
```

## Params

### READS_MIN_LEN (default: 3000)
All raads shorter than value set here will be filtered at the very start.
It should be part of separate executable but it lives here for the sake of fast development.

### OVERLAPS_MIN_QUALITY (default: 0.0 => not filtering)
All overlaps having quality less than the number defined here will be filtered at the very start.
Different overlap formats will have different ways of expressing this value or its derivatives so
better contact source or leave it as it is.

### READ_LEN_THRESHOLD (default: 100000)
During trimming step, if read is longer than this value, it won't be considered as a potential start of some tip.
Leaving that as it is would be a smart move.

### MAX_READS_IN_TIP (default: 2)
During trimming step, if tip has more than this nimber, it won't be cut.
Leaving that as it is would be a smart move if you do not know what you're doing.

### MAX_DEPTH_WITHOUT_EXTRA_FORK (default: 5)
During trimming step, detecting tips could be difficult. This parameter prevents considering something as a tip too early.
Leaving that as it is would be a smart move if you do not know what you're doing.

### MAX_NODES (default: 160)
During bubble popping, we have to stop the bubble searching when the nodes pool is too big (for the sake of efficiency).
Default value should be really fine for most of the assemblies, it is very probable you will never need more.

### MAX_DIFFERENCE (default: 0.25, percentage)
During bubble popping, it is very rare (read **never**) that two bubble paths will be exactly the same. This number says the maximum
edit distance two bubble paths can have (percentage) to be considered as parts of the same bubble.

Default value is fine for Oxford Nanopore reads, but not for PacBio (should be lower).

It is preferrable to have it slightly higher than expected error rate.

### MAX_BRANCHES (default: 18)
During contig extraction, exploration of paths will stop after this number of path forks.

Be aware that increasing this number could exponentially increase the time of contig extraction.

### MAX_START_NODES (default: 100)
Before contig extraction, number of nodes that are considered as good starting points.

### ~~LENGTH_THRESHOLD (default: 0.05, percentage)~~

### QUALITY_THRESHOLD (default: 0.2, percentage)
During contig extraction, overlap with quality in `[(1-QUALITY_THRESHOLD) * best_quality, best_quality]`
range is considered as the next one (wins one that produces the longest final path).
`best_quality` is the maximum quality of overlaps associated to a particular read.
