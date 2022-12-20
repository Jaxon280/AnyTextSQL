package edu.utokyo.vlex;

public class VlexNative {

    static {
        System.loadLibrary("vlex"); // Load native library at runtime
    }

    public native String patternToSchema(String pattern);
}