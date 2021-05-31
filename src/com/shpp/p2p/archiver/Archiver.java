package com.shpp.p2p.archiver;

import java.io.*;
import java.util.LinkedList;

/**
 * This module is responsible for packing/unpacking files,
 * as well as updating basic information about these operations in main class
 */
public class Archiver implements Constants {

    private static long                 startTime;    /* Start time in milliseconds */
    private static BufferedInputStream  input;        /* Buffered input */
    private static BufferedOutputStream output;       /* Buffered output */
    public  static byte[]               buffer;       /* Buffer for storing temporary data (parts of files) */
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
        heading.signature = ARCH_SIGNATURE;

        if (VERBOSE)
            System.out.println("Compressing the file: " + fileIn + "\n\n" + "Saving to file: " + fileOut + "\n");

        /* Otherwise return failure if exception occurs */
        try {
            if (!isFileCorrect(fileIn)) {
                System.out.println("Incorrect file: contains less than 2 unique bytes");
                return FAILURE;
            }
            EncodingTreeNode tree = ArchiveHeading.buildHuffmanTree(fileIn);
            flattenTree(tree);
            writeHeading();
            Sequence[] map = new Sequence[MAX_UNIQUE_BYTES];
            map = buildMap(tree, map, 0, 0);
            heading.ignoreBits = compressInBlocks(map);
        } catch (Exception e) {
            e.printStackTrace();
            return FAILURE;
        } finally {
            post(fileOut);
        }

        /* Rewriting first heading field */
        overwriteIgnoreBits(fileOut);
        return 0;
    }

    /**
     * Determines the number of unique bytes in file
     * Returns true if the file has 2+ unique bytes
     */
    private static boolean isFileCorrect(String fileIn) {
        try (BufferedInputStream readBytes = new BufferedInputStream(new FileInputStream(fileIn))) {
            int firstByte  = readBytes.read();
            int secondByte = firstByte;

            while (secondByte != -1) {
                if (firstByte != secondByte)
                    return true;

                // Reads next byte from the file
                secondByte = readBytes.read();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return false;
    }

    /**
     * Given a huffman encoding tree, flattens it and initializes
     * heading.treeShape, treeLeaves, and their sizes
     */
    private static void flattenTree(EncodingTreeNode tree) {
        heading.treeShape  = new LinkedList<>();
        heading.treeLeaves = new LinkedList<>();

        heading.treeShape  = flattenShape(tree, heading.treeShape);
        heading.treeLeaves = flattenLeaves(tree, heading.treeLeaves);

        /* Initialize tree shape size in bytes */
        heading.treeShapeSize = (byte) (heading.treeShape.size() / Byte.SIZE);
        if (heading.treeShape.size() % Byte.SIZE != 0) /* Round up if there's a remainder */
            heading.treeShapeSize++;

        /* Initialize tree leaves size in bytes */
        heading.treeLeavesSize = heading.treeLeaves.size() - 1;
    }

    /**
     * Flattens tree structure
     */
    private static LinkedList<Boolean> flattenShape(EncodingTreeNode tree, LinkedList<Boolean> shape) {
        if (tree.hasValue) { /* Tree leaf */
            shape.add(false);
            return shape;
        }
        /* Otherwise, it's a parent node */
        shape.add(true);
        shape = flattenShape(tree.left, shape);
        shape = flattenShape(tree.right, shape);
        return shape;
    }

    /**
     * Flattens tree leaves
     */
    private static LinkedList<Byte> flattenLeaves(EncodingTreeNode tree, LinkedList<Byte> leaves) {
        if (tree.hasValue) {
            leaves.add((byte) tree.uniqueByte);
            return leaves;
        }
        /* Otherwise, it's a parent node */
        leaves = flattenLeaves(tree.left, leaves);
        leaves = flattenLeaves(tree.right, leaves);
        return leaves;
    }

    /**
     * Writes heading to archive file
     */
    private static void writeHeading() throws IOException {
        /* One empty byte, will be overwritten in the end of the program */
        output.write(0);

        /* Other fields */
        output.write(heading.signature);
        output.write(heading.treeShapeSize);
        output.write(heading.treeLeavesSize);

        /* Writing tree shape bytes */
        for (int i = 0; i < heading.treeShapeSize; i++) {
            int currentByte = 0;
            /* Generating each byte from 8 bits */
            for (int j = 0; j < Byte.SIZE; j++) {
                Boolean currentBit = heading.treeShape.poll();
                if (currentBit == null)
                    currentBit = false;
                if (currentBit)
                    currentByte |= (1 << (Byte.SIZE - 1 - j));
            }
            /* Write current byte */
            output.write(currentByte);
        }

        /* Writing tree leaves bytes */
        while (!heading.treeLeaves.isEmpty())
            output.write(heading.treeLeaves.poll());
    }

    /**
     * Given a huffman encoding tree, builds a map of all leaves and their values.
     * An array of ByteSequences (size = 256) represents the map. The index of each element
     * is a unique byte value. The value of each element is its encoded sequence of bits
     *
     * currentSeq - current sequence, which we are forming for the next leaf
     * size       - current sequence size
     */
    private static Sequence[] buildMap(EncodingTreeNode tree, Sequence[] map, int currentSeq, int size) {
        if (tree.hasValue) { /* We reached a tree leaf */
            map[tree.uniqueByte] = (new Sequence(currentSeq, size));
            return map;
        }
        /* Otherwise, we have a parent node */
        currentSeq <<= 1; /* Shifting current path sequence one cell left */
        size++;           /* Incrementing size of sequence */

        /* First, moving left */
        map = buildMap(tree.left, map, currentSeq, size);

        /* Then, moving right */
        currentSeq |= 1;  /* Right => sequence has "1" added */
        map = buildMap(tree.right, map, currentSeq, size);

        return map;
    }

    /**
     * Given a buffer of bytes with a specified block size, compresses info there according to the
     * association table and the seqSize and writes it into the output stream
     *
     * @param map - Huffman encoding tree map
     * @return - Number of additional bits added to the archive
     */
    private static int compressInBlocks(Sequence[] map) throws IOException {
        int current;                                        /* Current byte to be written */
        int replaceValIndex = 0;                            /* Index of current bit in replace value */
        int bufferIndex     = 0;                            /* Index of current byte in buffer */
        int size = updateBuffer();                          /* Update buffer and get the size of bytes read from file */
        Sequence currentReplaceVal = map[buffer[0] & 0xff]; /* Replace bit combination of current byte in buffer */

        int i = 0; /* Iteration variable */

        /*
         * Each iteration:
         * 1. Form byte
         * 2. Write byte to output stream
         */
        while (size != -1) {
            current = 0;

            /* Forming a byte */
            for (i = 0; i < Byte.SIZE; i++) {

                /* Adding one bit a time */
                int mask = 1 << (currentReplaceVal.size - 1 - replaceValIndex);
                int currentBit = currentReplaceVal.value & mask;

                /* Adding bit to the byte, which is being archived currently */
                current |= ((currentBit >> (currentReplaceVal.size - 1 - replaceValIndex)) << (Byte.SIZE - 1 - i));

                /* Moving to next byte in the buffer of initial bytes */
                if (currentReplaceVal.size - 1 - replaceValIndex == 0) {
                    bufferIndex++;
                    replaceValIndex = 0;

                    /* Update buffer if it ended */
                    if (bufferIndex == size) {
                        size = updateBuffer();
                        if (size == -1) /* If end of file */
                            break;
                        bufferIndex = 0;
                    }

                    currentReplaceVal = map[buffer[bufferIndex] & 0xff];
                }
                /* Otherwise, moving to the next bit in current value associated bit consequence */
                else
                    replaceValIndex++;
            }

            output.write(current);
        }
        return (Byte.SIZE - (i + 1)); /* Number of additional zeros in the end of file */
    }

    /**
     * Rewrites the first byte in the archive file with ignoreBits
     */
    private static void overwriteIgnoreBits(String fileOut) {
        try (RandomAccessFile file = new RandomAccessFile(fileOut, "rw")) {
            file.seek(0);
            file.writeByte(heading.ignoreBits);
        } catch (IOException e) {
            e.printStackTrace();
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
            /* Read first two heading field */
            int ignoreBits = input.read();
            int signature  = input.read();

            /* Check signature */
            if (signature != ARCH_SIGNATURE) {
                System.out.println("Error: invalid archive");
                return FAILURE;
            }

            /* Read last four heading fields */
            int treeShapeSize  = input.read();
            int treeLeavesSize = input.read() + 1;
            EncodingTreeNode tree = getTree(treeShapeSize, treeLeavesSize);

            /* Decompress file */
            decompressInBlocks(tree, ignoreBits);

        } catch (Exception e) {
            e.printStackTrace();
            return FAILURE;
        } finally {
            post(fileOut);
        }
        return 0;
    }

    /**
     * Given the encoding tree shape and leaves size, read it from the file,
     * unflattens and returns the result
     */
    private static EncodingTreeNode getTree(int shapeSize, int leavesSize) throws IOException {
        LinkedList<Boolean> shape  = new LinkedList<>();  /* Tree shape */
        LinkedList<Byte>    leaves = new LinkedList<>();  /* Tree leaves */

        /* Read tree shape */
        for (int i = 0; i < shapeSize; i++) {
            int currentByte = input.read();

            /* Generating 8 bits and put them to shape array */
            for (int j = 0; j < Byte.SIZE; j++) {
                int currentBit = (currentByte >> (Byte.SIZE - j - 1)) & 1;
                if (currentBit == 1)
                    shape.add(true);
                else
                    shape.add(false);
            }
        }

        /* Read tree leaves */
        for (int i = 0; i < leavesSize; i++)
            leaves.add((byte) input.read());

        return unflattenTree(shape, leaves);
    }

    /**
     * Given tree shape and leaves, unflattens tree and returns it
     */
    private static EncodingTreeNode unflattenTree(LinkedList<Boolean> treeShape, LinkedList<Byte> treeLeaves) {
        Boolean currentBit = treeShape.getFirst();
        if (currentBit == null)
            return null;
        treeShape.removeFirst();

        /* Current bit indicates it's a leaf node */
        if (!currentBit) {
            EncodingTreeNode leaf = new EncodingTreeNode();
            leaf.uniqueByte = treeLeaves.getFirst();
            leaf.hasValue = true;
            treeLeaves.removeFirst();
            return leaf;
        }

        /* Otherwise we have a parent node */
        EncodingTreeNode parent = new EncodingTreeNode();
        parent.hasValue = false;
        parent.left  = unflattenTree(treeShape, treeLeaves);
        parent.right = unflattenTree(treeShape, treeLeaves);
        return parent;
    }


    /**
     * Given a buffer of bytes with a specified blocks size, decompresses info there according to the
     * huffman encoding tree and writes it into the output stream
     *
     * @param tree - Huffman encoding tree
     * @param ignoreBits - Bits to ignore in the last element of buffer (to cut redundant bytes in the end of file)
     */

    private static void decompressInBlocks(EncodingTreeNode tree, int ignoreBits) throws IOException {
        byte bitIndex = 0;          /* Index of current bit in buffer's byte (0-7) */
        int  bufferIndex = 0;       /* Index of current byte in buffer */
        int  size = updateBuffer(); /* Update buffer and get the size of bytes read from file */

        /*
         * Each iteration:
         * 1. Going through Huffman tree until reaching leaf
         * 2. Get leaf's value
         * 2. Write it to output stream
         */
        while (size != -1) {
            EncodingTreeNode currentNode = tree;

            /* Forming unique combination and fetching its value from Huffman tree */
            while (!currentNode.hasValue) {

                /* Fetching current bit from the buffer */
                int currentBit = ((0b10000000 >> bitIndex) & buffer[bufferIndex]) >> (Byte.SIZE - 1 - bitIndex);

                /* Check the value (either 1 or 0) */
                if (currentBit == 0)
                    currentNode = currentNode.left;
                else
                    currentNode = currentNode.right;

                /* Check if we should ignore all the next bits in this byte */
                if (bufferIndex == size - 1 && input.available() <= 0 && bitIndex == Byte.SIZE - 1 - ignoreBits) {
                    size = -1;
                    break;
                }

                /* Moving to the next byte in buffer */
                if (bitIndex == Byte.SIZE - 1) {
                    bufferIndex++;
                    bitIndex = 0;

                    /* Update buffer if it ended */
                    if (bufferIndex == size) {
                        size = updateBuffer();
                        bufferIndex = 0;
                    }
                }
                /* Otherwise, moving to the next bit in current buffer's byte */
                else
                    bitIndex++;
            }

            /* We've found the value of current bit combination, writing it */
            output.write(currentNode.uniqueByte);
        }
    }


    ////////////////////////////////////////////
    ////                                    ////
    ////          SHARED METHODS            ////
    ////                                    ////
    ////////////////////////////////////////////


    /**
     * Refreshes buffer, returns the number of bytes read into it
     * Returns -1 when nothing is available for reading from the file
     */
    private static int updateBuffer() throws IOException {
        return (input.read(buffer, 0, BLOCK_SIZE));
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
    private static void post(String fileOut) {
        /* Close streams */
        try {
            input.close();
            output.flush();
            output.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        Main.time = (double) (System.currentTimeMillis() - startTime) / 1000;
        File f = new File(fileOut);

        Main.fileOutSize = f.length();
        Main.efficiency = ((double) Main.fileOutSize / Main.fileInSize) * 100;
    }

}