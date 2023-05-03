package edu.utokyo.vlex;

import java.nio.ByteBuffer;

public class VlexNative {
    static {
        System.loadLibrary("vlex"); // Load native library at runtime
    }

    public native long parse(ByteBuffer addr, int sizeInRow, int colSize, int varSize, String command, int command_length, String query, int query_length);
}