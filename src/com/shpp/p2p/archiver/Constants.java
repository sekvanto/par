package com.shpp.p2p.archiver;

public interface Constants {

    /* Exit failure */
    int FAILURE = -1;

    /* Default input file */
    String DEFAULT_FILEIN = "test.txt";

    /* Archive signature */
    byte ARCH_SIGNATURE = 0x3a;

    /* Print extra info? */
    boolean VERBOSE = true;

    /* The number of all possible unique bytes */
    int MAX_UNIQUE_BYTES = 256;

    /* Block size in bytes */
    int BLOCK_SIZE = 65536;

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
