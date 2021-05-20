package com.shpp.p2p.archiver;

import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * This module is responsible for packing/unpacking files,
 * as well as updating basic information about these operations in main class
 */
public class Archiver implements Constants {

    private static long                 startTime;    /* Start time in milliseconds */
    private static BufferedInputStream  input;        /* Buffered input */
    private static BufferedOutputStream output;       /* Buffered output */
    private static byte[]               buffer;       /* Buffer for storing temporary data (parts of files) */
    private static ArchiveHeading       heading;      /* Archive heading */


    ////////////////////////////////////////////
    ////                                    ////
    ////             ARCHIVE                ////
    ////                                    ////
    ////////////////////////////////////////////


    /**
     * Takes names of two files, compresses input file and saves result to output file
     * returns code of success (0 = successful)
     */
    public static int archive(String fileIn, String fileOut) {
        /* If input file doesn't exist */
        if (init(fileIn, fileOut) != 0)
            return FAILURE;

        /* Initialize archive heading */
        heading = new ArchiveHeading();
        heading.decompressedDataSize = Main.fileInSize;

        if (VERBOSE)
            System.out.println("Compressing the file: " + fileIn + "\n\n" + "Saving to file: " + fileOut + "\n");

        /* Otherwise return failure if exception occurs */
        try {
            /* Find table sequences size in bits */
            int seqSize = determineSequencesBitSize(fileIn); /* Side effect: initializes heading.table */
            writeHeading();
            compressFileInBlocks(seqSize);
            post(fileOut);
        } catch (Exception e) {
            e.printStackTrace();
            return FAILURE;
        }
        return 0;
    }

    /**
     * Determines the number of unique bytes in buffered input,
     * based on which determines the size of associated sequences in table in bits (1-8);
     * passes unique bytes to a function, which creates association codes table
     */
    private static int determineSequencesBitSize(String fileIn) {
        ArrayList<Byte> uniqueBytes = new ArrayList<>();
        try {
            BufferedInputStream readBytes = new BufferedInputStream(new FileInputStream(fileIn));
            int i = readBytes.read();
            while (i != -1) {
                if (!uniqueBytes.contains((byte) i))
                    uniqueBytes.add((byte) i);

                if (uniqueBytes.size() == 256) /* If all possible bytes were found in file */
                    break;

                // Reads next byte from the file
                i = readBytes.read();
            }
            readBytes.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        int bits = findNumberOfBits(uniqueBytes.size());

        if (VERBOSE) {
            System.out.println("Unique bytes: " + uniqueBytes.size() + "\n");
            System.out.println("Associated sequences in table will have " + bits + " bits.\n");
        }

        /* Pass key table size, unique bytes to initialize heading association table */
        heading.table = ArchiveHeading.createTable(uniqueBytes);
        heading.tableSize = heading.table.size();
        return bits;
    }

    /**
     * Writes heading to archive file
     */
    private static void writeHeading() throws IOException {
        /* Write tableSize bytes one by one */
        for (int i = 0; i < (BITS_IN_BYTE * 4); i += BITS_IN_BYTE) { /* 4 bytes in integer */
            output.write((byte) (heading.tableSize >> i));
        }

        /* Write decompressedDataSize bytes one by one */
        for (int i = 0; i < (BITS_IN_BYTE * 8); i += BITS_IN_BYTE) { /* 8 bytes in long */
            output.write((byte) (heading.decompressedDataSize >> i));
        }

        for (byte key : heading.table.keySet()) {
            output.write(key);
            output.write(heading.table.get(key));
        }
    }

    /**
     * Reads blocks one by one, compresses bytes in there, writes them to output stream
     */
    private static void compressFileInBlocks(int seqSize) throws IOException {
        do {
            int bytesRead = input.read(buffer, 0, BLOCK_SIZE); /* Reading next block into buffer */

            /* Compress block and write it to file */
            compressWriteBlock(bytesRead, BITS_IN_BYTE - seqSize);

        } while (input.available() != 0);
    }

    /**
     * Given a buffer of bytes with a specified size, compresses info there according to the
     * association table and the seqSize and writes it into the output stream
     *
     * offset - offset on the beginning of each unique shortcut combination in bits (= BITS_IN_BYTE - seqSize)
     */
    private static void compressWriteBlock(int size, int offset) throws IOException {
        byte current;                                           /* Current byte to be written */
        byte currentReplaceVal = heading.table.get(buffer[0]);  /* Replace bit combination of current byte in buffer */
        byte replaceValIndex   = 0;                             /* Index of current bit in replace value */
        int  bufferIndex       = 0;                             /* Index of current byte in buffer */

        /*
         * Each iteration:
         * 1. Form byte
         * 2. Write byte to output stream
         */
        while (bufferIndex != size) {
            current = 0;

            for (int i = 0; i < BITS_IN_BYTE; i++) {
                /* Adding one bit a time */
                int currentBit = (0b10000000 >>> (offset + replaceValIndex)) & currentReplaceVal;

                /* Adding bit to the byte, which is being archived currently */
                current |= ((currentBit << (offset + replaceValIndex)) >>> i);

                /* Moving to next byte in the buffer of initial bytes */
                if ((offset + replaceValIndex) == BITS_IN_BYTE - 1) {
                    bufferIndex++;
                    replaceValIndex = 0;

                    /* Test if possible to continue */
                    if (bufferIndex == size)
                        break;

                    currentReplaceVal = heading.table.get(buffer[bufferIndex]);
                }
                /* Otherwise, moving to the next bit in current value associated bit consequence */
                else
                    replaceValIndex++;
            }

            output.write(current);
        }
    }


    ////////////////////////////////////////////
    ////                                    ////
    ////            UNARCHIVE               ////
    ////                                    ////
    ////////////////////////////////////////////


    /**
     * Takes names of two files, decompresses input file and saves result to output file
     * returns code of success (0 = successful)
     */
    public static int unarchive(String fileIn, String fileOut) {
        /* If input file doesn't exist */
        if (init(fileIn, fileOut) != 0)
            return FAILURE;

        if (VERBOSE)
            System.out.println("Decompressing the file: " + fileIn + "\n\n" + "Saving to file: " + fileOut + "\n");

        /* Otherwise return failure if exception occurs */
        try {
            /* Read first heading field */
            int tableSize = readFourBytes();

            /* Check table validity to not get into endless loop later on */
            if (tableSize > 256) {
                System.out.println("Error: invalid archive");
                return FAILURE;
            }

            /* Read last two heading fields */
            long decompressedDataSize = readEightBytes();
            HashMap<Byte, Byte> table = getTable(tableSize);

            /* Decompress file */
            int seqSize = findNumberOfBits(tableSize);
            decompressFileInBlocks(seqSize, decompressedDataSize, table);

            /* Post-check output file size */
            post(fileOut);
            if (Main.fileOutSize != decompressedDataSize) {
                System.out.println("Decompressed file is " + Main.fileOutSize + " bytes, should be " +
                                    decompressedDataSize + " bytes.");
                return FAILURE;
            }

        } catch (Exception e) {
            e.printStackTrace();
            return FAILURE;
        }
        return 0;
    }

    /**
     * Repeatedly reads four bytes from input stream and returns them as int
     */
    private static int readFourBytes() throws IOException {
        int result = 0;
        for (int i = 0; i < (BITS_IN_BYTE * 4); i += BITS_IN_BYTE) { /* 4 bytes in integer */
            result |= (input.read() << i);
        }
        return result;
    }

    /**
     * Repeatedly reads eight bytes from input stream and returns them as long
     */
    private static long readEightBytes() throws IOException {
        long result = 0;
        for (int i = 0; i < (BITS_IN_BYTE * 8); i += BITS_IN_BYTE) { /* 8 bytes in long */
            result |= ((long) input.read() << i);
        }
        return result;
    }

    /**
     * Given the association table size, read it from the file,
     * generate a HashMap and return it
     */
    private static HashMap<Byte, Byte> getTable(int size) throws IOException {
        HashMap<Byte, Byte> result = new HashMap<>();

        /* Read (size) bytes from file and put them to HashMap */
        for (int i = 0; i < size; i++) {
            byte value = (byte) input.read();
            byte key   = (byte) input.read();
            result.put(key, value);
        }

        return result;
    }

    /**
     * Reads blocks one by one, decompresses bytes in there, writes them to output stream
     */
    private static void decompressFileInBlocks(int seqSize, long dataSize, HashMap<Byte, Byte> table) throws IOException {

        long bytesCounter = 0;

        do {
            int bytesRead = input.read(buffer, 0, BLOCK_SIZE); /* Reading next block into buffer */

            /* Decompress block and write it to file */
            bytesCounter += decompressWriteBlock(bytesRead, seqSize, dataSize - bytesCounter, table);

        } while (input.available() != 0);
    }

    /**
     * Given a buffer of bytes with a specified size, decompresses info there according to the
     * association table and the seqSize and writes it into the output stream
     *
     * seqSize  - bit sequence size (1-8)
     * table    - bit sequences association table
     * size     - buffer size
     * readable - bytes available for reading to not exceed decompressedDataSize
     *
     * returns number of bytes decompressed
     */
    private static long decompressWriteBlock(int size, int seqSize, long readable, HashMap<Byte, Byte> table) throws IOException {
        byte current;                          /* Current bit combination from file */
        byte bitIndex     = 0;                 /* Index of current bit in buffer's byte (0-7) */
        int  bufferIndex  = 0;                 /* Index of current byte in buffer */
        long bytesRead    = 0;                 /* The number of bytes read (need this to cut redundant bytes in the end of file) */

        /*
         * Each iteration:
         * 1. Form bit combination
         * 2. Get initial byte from table
         * 2. Write byte to output stream
         */

        while (bufferIndex != size) {
            current = 0;

            for (int i = 0; i < seqSize; i++) {

                /* Adding one bit at a time */
                int currentBit = (0b10000000 >>> bitIndex) & buffer[bufferIndex];

                /* Adding bit to the bit combination, which we are forming currently */
                current |= ((currentBit << bitIndex) >>> ((BITS_IN_BYTE - seqSize) + i));

                /* Moving to the next byte in buffer */
                if (bitIndex == BITS_IN_BYTE - 1) {
                    bufferIndex++;
                    bitIndex = 0;

                    /* Test if possible to continue */
                    if (bufferIndex == size)
                        break;
                }
                /* Otherwise, moving to the next bit in current buffer's byte */
                else
                    bitIndex++;
            }
            if (readable == 0) /* Don't write more bytes if exceeds the number of readable ones */
                break;

            readable--;  /* One more byte was read, so reducing the number of readable ones */
            bytesRead++; /* One more byte was read */

            output.write(table.get(current));
        }
        return bytesRead;
    }


    ////////////////////////////////////////////
    ////                                    ////
    ////          SHARED METHODS            ////
    ////                                    ////
    ////////////////////////////////////////////


    /**
     * For a given number of possible combination, finds the number
     * of bits, required to store all of them
     */
    private static int findNumberOfBits(int combinations) {
        int result;
        double powOfTwo = Math.log(combinations) / Math.log(2);
        if (powOfTwo % 1 == 0)          /* If integer */
            result = (int) powOfTwo;
        else
            result = (int)(powOfTwo + 1); /* Rounding up, ex. 11 unique bytes (2^(3.46)) require 4 bits */
        if (result == 0)                  /* Minimum bits number will always be 1 */
            result = 1;
        return result;
    }


    /**
     * Runs on the start of the program, initializes start time, input file size, streams etc
     */
    private static int init(String fileIn, String fileOut) {
        startTime = System.currentTimeMillis();
        File f = new File(fileIn);
        if (!f.exists()) {
            System.out.println("Input file doesn't exist\n");
            return FAILURE;
        }

        /* Check if file is empty */
        if (f.length() == 0) {
            System.out.println("Input file is empty\n");
            return FAILURE;
        }

        Main.fileInSize = f.length();

        /* Initialize input/output stream */
        try {
            input  = new BufferedInputStream(new FileInputStream(fileIn), BLOCK_SIZE);
            output = new BufferedOutputStream(new FileOutputStream(fileOut), BLOCK_SIZE); /* Overwrite any existing file */
        } catch (FileNotFoundException e) {
            return FAILURE;
        }

        /* Initialize buffer for storing temporary data */
        buffer = new byte[BLOCK_SIZE];
        return 0;
    }

    /**
     * Runs in the end of the program, closes streams, initializes end time etc
     */
    private static void post(String fileOut) throws IOException {
        /* Close streams */
        input.close();
        output.flush();
        output.close();

        Main.time = (double) (System.currentTimeMillis() - startTime) /1000;
        File f = new File(fileOut);

        Main.fileOutSize = f.length();
        Main.efficiency = ((double) Main.fileOutSize / Main.fileInSize) * 100;
    }

}