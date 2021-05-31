package com.shpp.p2p.archiver;

/**
 * Represents a single node of encoding huffman tree
 */
public class EncodingTreeNode implements Comparable<EncodingTreeNode>  {
    int  uniqueByte;
    long weight;
    boolean hasValue;

    EncodingTreeNode left;
    EncodingTreeNode right;

    /**
     * Initializing end node with a value - this method is intended solely for building the tree
     */
    EncodingTreeNode(long weight, int uniqueByte) {
        this.uniqueByte = uniqueByte;
        this.weight = weight;
        hasValue = true;
        right = null;
        left = null;
    }

    /**
     * Initializing node without a value
     */
    EncodingTreeNode(long weight) {
        this.weight = weight;
        hasValue = false;
        right = null;
        left = null;
    }

    /**
     * Initializing a fully empty node
     */
    EncodingTreeNode() {
        right = null;
        left  = null;
    }

    /**
     * Overrides the method for comparing two objects in priority queue
     * based on their weight
     */
    @Override
    public int compareTo(EncodingTreeNode e) {
        return e.weight < this.weight ? 1 : -1; /* In ascending order */
    }

}