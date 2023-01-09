/*
 * Copyright 2014 Databricks
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package edu.utokyo.vlex

import java.io.{IOException, ObjectInputStream, ObjectOutputStream}

import org.apache.hadoop.conf.Configuration
import org.apache.hadoop.fs.{FileStatus, Path}
import org.apache.hadoop.mapreduce.{Job, TaskAttemptContext}
import org.apache.spark.sql.SparkSession
import org.apache.spark.sql.catalyst.InternalRow
import org.apache.spark.sql.catalyst.util.{CaseInsensitiveMap, CompressionCodecs}
import org.apache.spark.sql.execution.datasources._
import org.apache.spark.sql.execution.datasources.text.TextFileFormat
import org.apache.spark.sql.sources.{DataSourceRegister, Filter}
import org.apache.spark.sql.types._
import org.slf4j.LoggerFactory

import scala.util.control.NonFatal

private[vlex] class DefaultSource extends TextFileFormat with DataSourceRegister {

  override def equals(other: Any): Boolean = other match {
    case _: DefaultSource => true
    case _ => false
  }

  override def shortName(): String = "vlex"


  private def verifySchema(schema: StructType): Unit = {
  }

  // For now, default schema is a single column of type Long
  override def inferSchema(
                            sparkSession: SparkSession,
                            options: Map[String, String],
                            files: Seq[FileStatus]): Option[StructType] = Some(new StructType().add("value", LongType))


  override def prepareWrite(
                             sparkSession: SparkSession,
                             job: Job,
                             options: Map[String, String],
                             dataSchema: StructType): OutputWriterFactory = {
    verifySchema(dataSchema)

    val textOptions = new TextOptions(options)
    val conf = job.getConfiguration

    textOptions.compressionCodec.foreach { codec =>
      CompressionCodecs.setCodecConfiguration(conf, codec)
    }

    new OutputWriterFactory {
      override def newInstance(
                                path: String,
                                dataSchema: StructType,
                                context: TaskAttemptContext): OutputWriter = {
        new TextOutputWriter(path, dataSchema, context)
      }

      override def getFileExtension(context: TaskAttemptContext): String = {
        ".txt" + CodecStreams.getCompressionExtension(context)
      }
    }
  }

  override def buildReader(
                            sparkSession: SparkSession,
                            dataSchema: StructType,
                            partitionSchema: StructType,
                            requiredSchema: StructType,
                            filters: Seq[Filter],
                            options: Map[String, String],
                            hadoopConf: Configuration): PartitionedFile => Iterator[InternalRow] = {

    // val broadcastedHadoopConf =
    //   sparkSession.sparkContext.broadcast(new SerializableConfiguration(hadoopConf))

    /*
     * From the UnsafeRow documentation:
     * Each tuple has three parts: [null bit set] [values] [variable length portion]
     *
     * The bit set is used for null tracking and is aligned to 8-byte word boundaries.  It stores
     * one bit per field.
     *
     * In the `values` region, we store one 8-byte word per field. (TODO: we currently don't do this; should we?)
     * For fields that hold fixed-length primitive types, such as long, double, or int, we store the value
     * directly in the word. For fields with non-primitive or variable-length values, we store a relative
     * offset (w.r.t. the base address of the row) that points to the beginning of the variable-length
     * field, and length (they are combined into a long).
     */
    val numFields = dataSchema.fields.length
    println("Num fields: " + numFields)

    val varSizes : List[Int] = List()
    dataSchema.fields.map { field =>
      field.dataType match {
        case StringType => varSizes :+ field.metadata.getLong("length").toInt
        case _ => varSizes
      }
    }
    val recordSizeInBytes = dataSchema.fields.map { field =>
      field.dataType match {
        case ByteType => 1
        case ShortType => 2
        case IntegerType => 4
        case FloatType => 4
        case BooleanType => 4
        case LongType => 8
        case DoubleType => 8
        case StringType => field.metadata.getLong("length").toInt + 8
        case _ =>
          throw new RuntimeException(field.dataType + " not supported in Vlex!")
      }
    }.sum + ((numFields >> 6) + 1) * 8 // Additional bytes to track null bits set at beginning of record
    println("Record size: " + recordSizeInBytes)

    (file: PartitionedFile) => {
      println(file.filePath)
      val patternStr = options("pattern")
      val isKeys: Boolean = options("keyOption").equals("-k")
      val queryStr = options("query")
      val vlex = new Vlex(numFields, recordSizeInBytes, varSizes.length)
      vlex.parse(file.filePath, patternStr, isKeys, queryStr)
      vlex.iterator()
    }
  }
}

class TextOutputWriter(
                        path: String,
                        dataSchema: StructType,
                        context: TaskAttemptContext)
  extends OutputWriter {

  private val writer = CodecStreams.createOutputStream(context, new Path(path))

  override def write(row: InternalRow): Unit = {
    if (!row.isNullAt(0)) {
      val utf8string = row.getUTF8String(0)
      utf8string.writeTo(writer)
    }
    writer.write('\n')
  }

  override def close(): Unit = {
    writer.close()
  }
}

/**
  * Options for the Text data source.
  */
private[vlex] class TextOptions(@transient private val parameters: CaseInsensitiveMap[String])
  extends Serializable {

  import TextOptions._

  def this(parameters: Map[String, String]) = this(CaseInsensitiveMap(parameters))

  /**
    * Compression codec to use.
    */
  val compressionCodec = parameters.get(COMPRESSION).map(CompressionCodecs.getCodecClassName)
}

private[vlex] object TextOptions {
  val COMPRESSION = "compression"
}


private[vlex]
class SerializableConfiguration(@transient var value: Configuration) extends Serializable {
  @transient private[vlex] lazy val log = LoggerFactory.getLogger(getClass)

  /**
    * Execute a block of code that returns a value, re-throwing any non-fatal uncaught
    * exceptions as IOException. This is used when implementing Externalizable and Serializable's
    * read and write methods, since Java's serializer will not report non-IOExceptions properly;
    * see SPARK-4080 for more context.
    */
  def tryOrIOException[T](block: => T): T = {
    try {
      block
    } catch {
      case e: IOException =>
        log.error("Exception encountered", e)
        throw e
      case NonFatal(e) =>
        log.error("Exception encountered", e)
        throw new IOException(e)
    }
  }

  private def writeObject(out: ObjectOutputStream): Unit = tryOrIOException {
    out.defaultWriteObject()
    value.write(out)
  }

  private def readObject(in: ObjectInputStream): Unit = tryOrIOException {
    value = new Configuration(false)
    value.readFields(in)
  }
}