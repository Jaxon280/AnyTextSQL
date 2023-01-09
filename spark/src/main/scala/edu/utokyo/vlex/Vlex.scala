package edu.utokyo.vlex

import java.nio.{ByteBuffer, ByteOrder}

import org.apache.spark.sql.catalyst.InternalRow
import org.apache.spark.sql.catalyst.expressions.UnsafeRow

class Vlex(val numFields: Int = 1, val recordSizeInBytes: Int = 16, val varSize: Int = 0, val maxRecords: Long = 16777216) {
  val vlexNative = new VlexNative()
  var rawAddress: Long = 0L
  var recordsParsed: Long = 0L
  val buf: ByteBuffer = ByteBuffer.allocateDirect((maxRecords * recordSizeInBytes).toInt)
  buf.order(ByteOrder.nativeOrder())

  def parse(filename: String, pattern: String, isKeys: Boolean, query: String): Unit = {
    rawAddress = RawMemory.getRawPointer(buf)
    println("In Scala, the address is " + "0x%08x".format(rawAddress))
    println(buf.position(), buf.limit())
    recordsParsed = vlexNative.parse(filename, filename.length,
      buf, recordSizeInBytes, varSize, pattern, pattern.length, isKeys, query, query.length)
    println(buf.position(), buf.limit())
    println("In Scala, records parsed: " + recordsParsed)
  }

  def iterator(): Iterator[InternalRow] = {
    new Iterator[InternalRow]() {

      val currRecord = new UnsafeRow(numFields)
      var currRecordIndex: Long = 0

      override def hasNext(): Boolean = {
        currRecordIndex < recordsParsed
      }

      override def next(): UnsafeRow = {
        currRecord.pointTo(
          null,
          rawAddress + currRecordIndex * recordSizeInBytes,
          recordSizeInBytes.toInt)
        currRecordIndex += 1
        currRecord
      }
    }
  }
}