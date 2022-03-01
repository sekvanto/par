par - pocket archiver. A simple data compression cli program, which supports a set of compression algorithms:

* Huffman coding
* Yet to be added

# Installation

Clone the repository
```
git clone https://github.com/sekvanto/par
cd par
```
Compile
```
make
```
Run
```
./archive [arguments]
```
To clean the source directory, run
```
make clean
```

# Example

![example](examples/example.png)

# Usage

## Synopsis

```
./archive [options] [input file] [output file]
```

## Options

Optional flags:

`-a` Explicit flag to forcefully archive first file into second file

`-u` Explicit flag to unarchive


If zero filenames are specified, program archives the default file ("test.txt").

If only one filename is specified, the output file name is generated automatically, e.g. Input = "file.txt" => Output = "file.txt.par". If input name has ".par" extension, file will be decompressed and gain extension ".uar", e.g. Input = "file.txt.par" => Output = "file.txt.uar".

If two filenames are specified, first file is compressed/decompressed into second file. Function will depend either on flags, or on input filename.

# FAQ

## How efficient is this archiver?

par v2.0 compresses 80 MB per second. The extent of compression depends on file - it's very efficient for files, which are long but have very few unique characters, but in the opposite case the efficiency will be worse.

# TODO

☑  Add Huffman coding support\
☑  Modify program structure, rewrite it from Java to C\
▢  Add adaptive Huffman coding (efficient for images), LZW\
▢  Add support for multithreading\
▢  Add a few lossy data compression algorithms (esp for multimedia)

# Architecture/technical details

See [Implementation details](https://github.com/sekvanto/par/wiki/Implementation-details) on project wiki.
