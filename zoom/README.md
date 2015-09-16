# zoom
Extract overlaps around given read id, up to the given depth.

## Usage

```
usage: bin/zoom --root=int --depth=int --overlaps=string [options] ...
options:
  -r, --root               root read (int)
  -n, --depth              neighborhood depth (int)
  -x, --overlaps           overlaps file (string)
  -f, --overlaps_format    overlaps file format; supported: afg, mhap (string [=afg])
  -?, --help               print this message
```

Example:
```
  ./bin/zoom -r 123 -d 5 -f mhap < overlaps.mhap | bin/overlap2dot -f mhap | neato -T svg -o graph.png
```
zooms area around read **123**, up to the depth of **5** (1 level = following 1 overlap).
