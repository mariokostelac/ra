# RA

RA is short for RNA Assembler and it is a C++ implementation of an overlap-layout-consensus transcriptome assembler. It was developed as part of my master's thesis at [FER](http://www.fer.unizg.hr).

## REQUIREMENTS
- g++ (4.6.3 or higher)
- GNU Make

\*note: It was only tested on Linux (Ubuntu).

## INSTALLATION

To build the RA project run the following commands from your terminal:

    git clone https://github.com/rvaser/ra.git ra
    cd ra/
    make

Running the 'make' command will create the bin folder where all executables will be stored.

## MODULES

Currently supported modules are:

1. [ra](ra/README.md) - Module is the main static library and is used by other modules (doesn't provide any executable).
2. [ra_overlap](ra_overlap/README.md) - Module is used for finding all overlaps between input single end reads. It also removes contained and transitive overlaps.
3. [ra_layout](ra_layout/README.md) - Module is used to create a string graph from input overlaps. It then simplifies it with trimming and bubble popping. At the end it extracts longest contigs from every graph component. As the current overlapper is exact, it also extracts whole transcripts (transcripts.layout.fasta) so that the consensus phase can be avoided for now.
4. [ra_consensus](ra_consensus/README.md) - Module is used to build consensus sequences, it uses [CPPPOA](https://github.com/mculinovic/cpppoa) and outputs transcripts.
5. [ra_correct](ra_correct/README.md) - Module is optional and is used to correct reads. If used, it should be called before ra_overlap.
6. [to_afg](to_afg/README.md) - Module is used for converting read sets from [FASTA][1]/[FASTQ][2] to [afg][3] format. It is neccessary to convert reads because all other modules are using the afg format.

## EXAMPLES

Convert reads:

    ./bin/to_afg -i examples/ERR430949.fastq --fastq > ERR430949.afg

Correct reads:

    ./bin/ra_correct -i ERR430949.afg -k 25 -c 2 > ERR430949_c.afg

Overlap phase:

    ./bin/ra_overlap -i ERR430949_c.afg -m 25 -t 10 --reads-out ERR430949_u.afg > ERR430949_ovl.afg

Layout phase:

    ./bin/ra_layout -i ERR430949_u.afg -j ERR430949_ovl.afg -t 10 > ERR430949_con.afg

Consensus phase (optional for now):

    ./bin/ra_consensus -i ERR430949_u.afg -j ERR430949_con.afg > transcripts.fasta


\*note: First ra_correct and ra_overlap runs for every set of reads will cache the enhanced suffix arrays for future usage and therefore will be slower (files with .cra, .nra and .rra extensions will be created). Next runs with the same input will use the cached files and will be faster.

[1]: https://en.wikipedia.org/wiki/FASTA_format "FASTA"
[2]: https://en.wikipedia.org/wiki/FASTQ_format "FASTQ"
[3]: http://www.amos.sourceforge.net/wiki/index.php/Message_Types "afg"
