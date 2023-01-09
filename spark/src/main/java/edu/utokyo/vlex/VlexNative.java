package edu.utokyo.vlex;

import java.nio.ByteBuffer;

public class VlexNative {
    static {
        System.loadLibrary("vlex"); // Load native library at runtime
    }

    public native long parse(String filename, int filename_length, ByteBuffer addr, int sizeInRow, int varSize, String pattern, int pattern_length, boolean isKeys, String query, int query_length);
}