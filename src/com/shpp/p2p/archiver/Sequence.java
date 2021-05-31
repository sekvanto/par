package com.shpp.p2p.archiver;

/**
 * An object of this class represents a byte sequence.
 * It's basically the same as Integer but with an additional size field
 * (for knowing which bits of the value field are included into the sequence)
 * (e.g. val = "0b00000000000000001", size = "3" => sequence = "001")
 */
public class Sequence {

    /* Value of the byte sequence */
    int value;

    /* Size of value in bits */
    int size;

    /* Constructor */
    Sequence(int value, int size) {
        this.value = value;
        this.size  = size;
    }

}