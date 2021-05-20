package com.shpp.p2p.archiver;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * An object of this class represents archive heading structure
 */
public class ArchiveHeading {

    /* 4 bytes - size of table in bytes */
    int  tableSize;

    /* 8 bytes - size of decompressed data in bytes */
    long decompressedDataSize;

    /* X bytes - association table */
    HashMap<Byte, Byte> table;

    /* Y bytes - archived data (no field) */
    /* ... */

    /**
     * Takes unique bytes, generates and returns association table for encoding
     * result: HashMap(initial byte, associated sequence)
     */
    public static HashMap<Byte, Byte> createTable(ArrayList<Byte> uniqueBytes) {
        HashMap<Byte, Byte> result = new HashMap<>();
        byte sequence = 0;
        for (byte inByte : uniqueBytes) {
            result.put(inByte, sequence);
            sequence++;
        }
        return result;
    }

}