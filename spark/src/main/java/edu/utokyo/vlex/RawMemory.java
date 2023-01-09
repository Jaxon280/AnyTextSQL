package edu.utokyo.vlex;

import java.lang.reflect.Field;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import sun.misc.Unsafe;

class RawMemory {

    private static Unsafe UNSAFE;
    private static long addressOffset;

    static {
        try {
            Field theUnsafe = Unsafe.class.getDeclaredField("theUnsafe");
            theUnsafe.setAccessible(true);
            UNSAFE = (Unsafe) theUnsafe.get(null);
            addressOffset = UNSAFE.objectFieldOffset(Buffer.class.getDeclaredField("address"));
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    static long getRawPointer(ByteBuffer buf) {
        return RawMemory.UNSAFE.getLong(buf, addressOffset);
    }
}