# ra_overlap

    usage: ra_overlap -i <reads file> [arguments ...]

    arguments:
        -i, --reads <file>
            (required)
            input afg reads file
        -t, --threads <int>
            default: approx. number of processors/cores
            number of threads used
        -m, --min-overlap-length <int>
            default: 5
            minimal length of exact overlap between two reads
        --reads-out <file>
            default: reads.afg
            output afg updated reads file
        --overlaps-out <file>
            default: overlaps.afg
            output afg overlaps file
        -h, -help
            prints out the help