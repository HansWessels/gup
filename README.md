[![autoconf CI: does it build?](https://github.com/HansWessels/gup/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/HansWessels/gup/actions/workflows/c-cpp.yml)
[![autoconf CI: running gup system tests](https://github.com/HansWessels/gup/actions/workflows/c-cpp-test.yml/badge.svg)](https://github.com/HansWessels/gup/actions/workflows/c-cpp-test.yml)
[![MSBuild/MSVC CI: does it build?](https://github.com/HansWessels/gup/actions/workflows/msbuild.yml/badge.svg)](https://github.com/HansWessels/gup/actions/workflows/msbuild.yml)

# GUP

## What's this, doc?

GUP is a data compressor, which can archive (= store a bundle of files & directories in a *compressed* container file). It can be used as an alternative for ARJ, RAR, ZIP, LZH (LHarc), ...

GUP is also your primary choice when you're looking for a packer that produces **tight packs** (a.k.a. high compression ratio), while also delivering **fast & *small* depacker code**.


## Target audience: Who, Me? 

*Bingo*, if:

- you are a 'scene coder' and looking for something *very neat* to pack all your code and data into a very small space (GUP hails from the hayday of Atari ST and Amiga, back in the 'naughties when MC68K was King, so we think we've got claim to some serious pedigree there, yes we do.)
- you are looking for a fast depacker.
- you are looking for tools which offer specialized enhancements and source code for **embedding the compressed data** and **swiftly depacking it in modern and classic generic *and* embedded hardware systems**. *Did we mention our depacker(s) are coded for both minimum size and maximum speed?*
- you are looking for tools that wring the very last drop of entropy from your collective data (highly compression-ratio-optimized LZ77+Huffman: better than ARJ/ZIP/RAR/LZH/...)[^1]
- you are curious what can be achieved with classic evergreen Lempel-Ziv sliding dictionary compression technology. We never published this stuff as a whitepaper, but, as of 2022AD, our extensive research has not uncovered anyone that does it better[^3] and when we do, be assured we'll be incorporating such cleverness in GUP!


## Which platforms do you support?

- Linux (autoconf/configure-script based build: *easy*)
- BSD (FreeBSD/OpenBSD/...) (autoconf/configure-script based build: *easy*)
- MS Windows (as of 2022AD we have MSVC2017 and MSVC2022 project files to help you build this stuff; MSVC2017 is going to be phased out sometime later)
- AtariST (we've got PureC GmbH (formerly BorlandC on AtariST) project files and hand-optimized high-speed assembly + C sources to go for you)
- MC68K hand-optimized assembly depackers available

This stuff is actively developed and tested on a Rasberry PI cluster (Linux), a huge AMD Ryzen *beast* (FreeBSD) and a couple of Microsoft Windows 10 desktop boxes (Intel core i7 and AMD Ryzen). The main developer also builds these goodies on classical AtariST hardware and in AtariST emulators.  We know of others using or having used the depacker(s) on Atari ST, Amiga, ...

The core decompressor code is available in both highly portable C and MC68K hand-optimized source code form; any 32bit or larger CPU should be doing quite well on this.

## Compression modes

- **ARJ m1, m2, m3** these all default to **ARJ m1** compression method using a 26 kB dictionary, a maximum match of 258 bytes and huffman encoding. This is compatible to ARJ by Robert K. Jung.
- **ARJ m4** is the ARJ m4 compression methode using a 16 kB dictionary and unary encoding of the dictionary references.
- **ARJ m7** is an extension on the **ARJ m1** format, It uses a 64 kB dictionary, a maximum match of 258 bytes and uses huffman encoding. m1, m2, m3 and m7 all can be depacked with the same decompression routine as long as the lager dictionary size is taken into account.
- **n0** is a lz77 packer using a 64 kB dictionary and a maximum match of 64 kB. It was written for a small depacker size and a high decompression speed.
- **n1** is a lz77 packer using a 128 kB dictionary and a maximum match of 64 kB. It is based on the ARJ m4 packer, using almost the same decompression routine only modified to allow for the larger dictionary size and larger matches.
- **n2** is a lz77 packer based on the 'nrv2s decompression in pure 68k asm by ross' code. I modified the format to increase depacking speed and compression ratio. It uses an (almost) 16 Mb dictionary. The maximum match is 64 kB.

### Compression performance

During development the compression performance is evaluated using multiple test data sets.
- **Calgary corpus** The [Calgary corpus](https://en.wikipedia.org/wiki/Calgary_corpus). I am using the 'less commonly used 18 file version which include 4 additional text files in UNIX "troff" format, PAPER3 through PAPER6' and it also includes the additional files 'readme', 'sizes' and 'sources' which came with the archive containing the Calgary corpus. The last three are small files and it is interesting to see how the various modes are doing with small files.
- **atari** a set of files comming from my Atari ST harddisk. It contains a lot of programs (.prg ) and Atari ST picture files (Neochrome and Degas format).
- **mame** a large set of Mame ROM dumps.
- **enwik8** The dataset is based on the [Hutter Prize](http://prize.hutter1.net) and contains the first 10^8 byte of Wikipedia

```
Filename:  original         m1         m7         m4         n0         n1         n2
bib          111261      35032      32722      39784       6288      35360      35661
book1        768771     310620     293575     344252     323797     321331     313451
book2        610856     204854     191955     226013     209978     204206     200727
geo          102400      67468      67211      70840      73291      70424      68711
news         377109     144004     135532     157940     145941     140302     136479
obj1          21504      10266      10266      10877      11168      10512       9965
obj2         246814      80182      78666      84570      83647      80166      75160
paper1        53161      18140      17840      19692      20359      19198      19302
paper2        82199      29066      28107      32133      31766      30753      30850
paper3        46526      17590      17459      19257      20138      18948      18953
paper4        13286       5402       5402       5921       6488       5855       5870
paper5        11954       4898       4898       5362       5857       5278       5299
paper6        38105      12996      12912      14123      14815      13852      13852
pic          513216      50591      50790      59042      58545      56135      55227
progc         39611      13005      12966      14153      14789      13941      13985
progl         71646      15835      15571      17131      17349      16621      16808
progp         49379      11056      10868      11849      11992      11443      11544
readme         2479       1219       1219       1480       1563       1396       1403
sizes           689        338        338        451        450        427        419
sources        1177        637        637        821        847        762        765
trans         93695      19459      18010      21870      19959      19029      19232

totaal      3255838    1052658    1006944    1157561    1109027    1075939    1053663

atari      86060507   35094890   34820856   38201619   38260071   36732301   35763556
mame     2687556853  895899395  886137249 1009435302  983897073  941651991  921598070
enwik8    100000000   36234895   34422639   39260290   36948297   35888724   32003287
```

### Compression and gup

Using the **gup** compression program which van be found in the **unix** directory when you build the project in a Unix environment, for windows the program can be found in the **win32** directory, ARJ files can be generated with a command line looking like this:  
`gup a -<mode> -e -r result.arj <file(s)>`  
Where **a** means create an ARJ file and **a**dd files to the ARJ file. **\<mode\>** is one of the compression modes: m1, m4, m7, n0, n1 or n2. The **-e** switch excludes the directory names in the ARJ file. The **-r** switch is for recursive directories.  
`gup l result.arj` will **l**ist the contents of the ARJ file.  
`gup t result.arj` will **t**est the contents of the ARJ file.  
`gup x result.arj` will e**x**tract the contents of the ARJ file.  

### Dump compressed files

The **gupdump** program will dump all the compressed files in an ARJ file in binary format. The program can be found in the **gupdump** directory on Unix systems or in the **win32** directory for windows. `gupdump result.arj` will dump all the files compressed in result.arj in binairy form. An extension is added identifying the compression mode used for the file: .m4, .m7, .n0, .n1 or .n2.  **gupdump** can also depack the dumped files. i.e. `gupdump readme.txt.m7` will try to depack the file readme.txt.m7 with ARJ mode 7 and write the result into the file readme.txt.

### Decompression

In de directory **ni-pack** decompression routines written in C can be found for all the compression modes. **depack_t.c** is a simple c file showing the usage of the decompression routines and can be used for timing the decompression speed.

## License

GUP is GPLv2. 

The depacker cores are MIT, so you can always use the packed data in *any* setting: both Open Source 🥳 and Commercial environments. 


### Rationale on License Intent & Commercial Usage

We want GUP to remain Open and Public and hope you will share your derivative works likewise. We also don't want any restrictions on the *use* of GUP; more specifically: the compressed data results.

Hence the choice for MIT for the **depacker**: this allows you, for example, to use the GUP tool in in-house commercial embedded development, write the compressed data *plus* our depacker code in ROM/Flash/... to save space and time and then sell your product. 

*Ditto* for UPX-style scenarios: you can *pack* your commercial software using GUP as an UPX alternative, for there again, your output is your own propriatary files in *compressed form*, plus our depacker code, to form a new executable that self-extracts and executes your code after depacking.

Meanwhile we want the **compressor** to remain open *forever*, including anyone riffing off it: that's what the GPL is for.

-------



[^1]: LZMA & tANS is considered for our Glorious Future, to exist next to the presently available LZ77 best-of-breeds implementation[^2].

[^2]: we care about compression ratio. GUP/ARJ mode 7 (LZ77+Huffman) and most of the other GUP compression modes' packer/**compressor** cores focus on (1) compression ratio and (2) compression speed; in that order. Meanwhile our depacker/**decompressor** is generally ARJ/LZH compatible and **fast**: we always aimed for Number 1 in the "pack once, depack many times" arena. Which, f.e., is what you need when writing a *scene demo* or other sophisticated software that benefits from both small distribution size and maximum features/content enjoyment.

[^3]: *better* meaning: higher performance in "pack once, depack many times" conditions: packing(=compression) speed is only a *secondary* focus here, while *highest possible compression ratio* is and always has been *primary focus* for GUP: we may be a bit slower when packing, but heck! we're giving you the smallest possible archives.
