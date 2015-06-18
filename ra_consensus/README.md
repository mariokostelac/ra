# ra_consensus

usage: ra_layout -i <reads file> -j <overlaps file> [arguments ...]

arguments:
    -i, --reads <file>
        (required)
        input afg reads file
    -j, --contigs <file>
        (required)
        input afg contigs file
    -t, --threads <int>
        default: approx. number of processors/cores
        number of threads used
    -o, --out <file>
        default: cout
        output fasta transcripts file
    -h, -help
        prints out the help
