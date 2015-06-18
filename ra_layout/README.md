# ra_layout

    usage: ra_layout -i <reads file> -j <overlaps file> [arguments ...]

    arguments:
        -i, --reads <file>
            (required)
            input afg reads file
        -j, --overlaps <file>
            (required)
            input afg overlaps file
        -t, --threads <int>
            default: approx. number of processors/cores
            number of threads used
        --contigs-out <file>
            default: contigs.afg
            output afg contigs file
        -h, -help
            prints out the help