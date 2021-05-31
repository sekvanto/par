package com.shpp.p2p.archiver;

import java.io.BufferedInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.LinkedList;
import java.util.PriorityQueue;
import java.util.Queue;

/**
 * An object of this class represents archive heading structure
 */
public class ArchiveHeading implements Constants {

    /* 1 byte - the number of additional bits to ignore in the end of arch. data */
    int ignoreBits;

    /* 1 byte - signature */
    byte signature;

    /* 1 byte - size of tree shape in bytes */
    byte treeShapeSize;

    /* 1 byte - size of tree leaves in bytes MINUS ONE (e.g. if size is 256, then 255 will be written) */
    int  treeLeavesSize;

    /* X bytes - huffman tree shape */
    LinkedList<Boolean> treeShape;

    /* Y bytes - huffman tree leaves */
    LinkedList<Byte> treeLeaves;

    /* Z bytes - archived data (no field) */
    /* ... */

    /**
     * Takes filename, generates and returns encoding tree
     */
    public static EncodingTreeNode buildHuffmanTree(String fileIn) {
        long[] weights = findBytesWeight(fileIn);
        Queue<EncodingTreeNode> forest = new PriorityQueue<>();
        for (int i = 0; i < weights.length; i++) {
            if (weights[i] == 0) /* Current byte has zero weight */
                continue;

            /* Creating initial set of nodes */
            forest.add(new EncodingTreeNode(weights[i], i));
        }
        while (forest.size() != 1) {
            EncodingTreeNode left  = forest.poll();
            EncodingTreeNode right = forest.poll();

            assert right != null;
            assert left  != null;
            EncodingTreeNode newNode = new EncodingTreeNode(left.weight + right.weight);
            newNode.left  = left;
            newNode.right = right;

            forest.add(newNode);
        }

        return forest.poll(); /* Returning last tree left in the forest */
    }

    /**
     * Takes filename, finds weight of each symbol in this file
     * Result: an array of size 256, where index of each element
     * represents a single unique byte; the value of each element
     * is weight (0 - file doesn't contain this byte)
     */
    private static long[] findBytesWeight(String fileIn) {
        long[] result = new long[MAX_UNIQUE_BYTES];

        try (BufferedInputStream readBytes = new BufferedInputStream(new FileInputStream(fileIn))) {
            while (true) {
                int bytesRead = readBytes.read(Archiver.buffer, 0, BLOCK_SIZE); /* Reading next block into buffer */

                if (bytesRead == -1)
                    break;

                /* Determine unique bytes in current block */
                for (int i = 0; i < bytesRead; i++) {
                    int index = Archiver.buffer[i] & 0xff; /* Cast byte to unsigned integer */
                    result[index]++;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

}