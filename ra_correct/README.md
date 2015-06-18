# ra_correct

    usage: to_afg -i <reads file> [arguments ...]

    arguments:
        -i, --reads <file>
            (required)
            input afg reads file
        -k, --kmer <int>
            default: based on dataset
            length of k-mers used in error correction
        -c, --threshold <int>
            default: based on dataset
            minimal number of occurrences for a k-mer to not be erroneous
        -o, --out <file>
            default: cout
            output afg corrected reads file
        -h, -help
            prints out the help