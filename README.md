[![autoconf CI: does it build?](https://github.com/HansWessels/gup/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/HansWessels/gup/actions/workflows/c-cpp.yml)
[![autoconf CI: running gup system tests](https://github.com/HansWessels/gup/actions/workflows/c-cpp-test.yml/badge.svg)](https://github.com/HansWessels/gup/actions/workflows/c-cpp-test.yml)
[![MSBuild/MSVC CI: does it build?](https://github.com/HansWessels/gup/actions/workflows/msbuild.yml/badge.svg)](https://github.com/HansWessels/gup/actions/workflows/msbuild.yml)

# GUP

## What's this, doc?

GUP is an packer, which can archive (= store a bundle of files & directories in a *compressed* container file). It can be used as an alternative for ARJ, RAR, ZIP, LZH (LHarc), ...

GUP is also your primary choice when you're looking for a packer that produces **tight packs** (a.k.a. high compression ratio), while also delivering **fast & *small* depacker code**.


## Target audience: Who, Me? 

*Bingo*, if:

- you are a 'scene coder' and looking for something *very neat* to pack all your code and data into a very small space (GUP hails from the hayday of Atari ST and Amiga, back in the 'naughties when MC68K was King, so we think we've got claim to some serious pedigree there, yes we do.)
- you are looking for a fast depacker.
- you are looking for tools which offer specialized enhacements and source code for **embedding the compressed data** and **swiftly depacking it in modern and classic generic *and* embedded hardware systems**. *Did we mention our depacker(s) are coded for both minimum size and maximum speed?*
- you are looking for tools that wring the very last drop of entropy from your collective data (highly compression-ratio-optimized LZ77+Huffman: better than ARJ/ZIP/RAR/LZH/...)[^1]
- you are curious what can be achieved with classic evergreen Lempel-Ziv sliding dictionary compression technology. We never published this stuff as a whitepaper, but, as of 2022AD, our extensive research has not uncovered anyone that does it better[^3] and when we do, be assured we'll be incorporating such cleverness in GUP!


## Which platforms do you support?

- Linux (autoconf/configure-script based build: *easy*)
- BSD (FreeBSD/OpenBSD/...) (autoconf/configure-script based build: *easy*)
- MS Windows (as of 2022AD we have MSVC2017 and MSVC2022 project files to help you build this stuff; MSVC2017 is going to be phased out sometime later)
- AtariST (we've got PureC GmbH (formerly BorlandC on AtariST) project files and hand-optimized high-speed assembly + C sources to go for you)
- MC68K hand-optimized assembly depackers available

This stuff is actively developed and tested on a Rasberry PI cluster (Linux), a huge AMD Ryzen *beast* (FreeBSD) and a couple of Microsoft Windows 10 desktop boxes (Intel core i7 and AMD Ryzen). The main developer also builds these goodies on classical AtariST hardware and in AtariST emulators.  We know of others using or having used the depacker(s) on AtariST, Amiga, ...

The core decompressor code is available in both highly portable C and MC68K hand-optimized source code form; any 32bit or larger CPU should be doing quite well on this.


## License

GUP is GPLv2. 

The depacker cores are MIT, so you can always use the packed data in *any* setting: both Open Source :yay: and Commercial environments. 


### Rationale on License Intent & Commercial Usage

We want GUP to remain Open and Public and hope you will share your derivative works likewise. We also don't want any restrictions on the *use* of GUP; more specifically: the compressed data results.

Hence the choice for MIT for the **depacker**: this allows you, for example, to use the GUP tool in in-house commercial embedded development, write the compressed data *plus* our depacker code in ROM/Flash/... to save space and time and then sell your product. 

*Ditto* for UPX-style scenarios: you can *pack* your commercial software using GUP as an UPX alternative, for there again, your output is your own propriatary files in *compressed form*, plus our depacker code, to form a new executable that self-extracts and executes your code after depacking.

Meanwhile we want the **compressor** to remain open *forever*, including anyone riffing off it: that's what the GPL is for.

-------



[^1]: LZMA & tANS is considered for our Glorious Future, to exist next to the presently available LZ77 best-of-breeds implementation[^2].

[^2]: we care about compression ratio. GUP/ARJ mode 7 (LZ77+Huffman) and most of the other GUP compression modes' packer/**compressor** cores focus on (1) compression ratio and (2) compression speed; in that order. Meanwhile our depacker/**decompressor** is generally ARJ/LZH compatible and **fast**: we always aimed for Number 1 in the "pack once, depack many times" arena. Which, f.e., is what you need when writing a *scene demo* or other sophisticated software that benefits from both small distribution side and maximum features/content enjoyment.

[^3]: *better* meaning: higher performance in "pack once, depack many times" conditions: packing(=compression) speed is only a *secondary* focus here, while *highest possible compression ratio* is and always has been *primary focus* for GUP: we may be a bit slower when packing, but heck! we're giving you the smallest possible archives.
