package com.shpp.p2p.archiver;

public interface Constants {

    /* Exit failure */
    int FAILURE = -1;

    /* Default input file */
    String DEFAULT_FILEIN = "test.txt";

    /* Print extra info? */
    boolean VERBOSE = true;

    /* Block size in bytes */
    int BLOCK_SIZE = 65536;

    /* Bits in one byte */
    int BITS_IN_BYTE = 8;

    /* Console options */
    String ARCHIVE   = "-a";
    String UNARCHIVE = "-u";

    /* File size units */
    String[] FSIZE_UNITS = new String[]{
            "B",
            "KB",
            "MB",
            "GB"
    };

}