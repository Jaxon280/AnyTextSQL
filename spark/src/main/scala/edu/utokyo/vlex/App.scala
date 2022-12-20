package edu.utokyo.vlex

import org.apache.spark.sql.SparkSession

object App {
    def main(args: Array[String]): Unit = {
        val spark = SparkSession.builder.appName("Sparser Spark").getOrCreate()
        val pattern: String = args(0)
        val queryStr: String = args(1)
        val filename: String = args(2)

        val vlexNative = new VlexNative()

        // parse pattern and create schema
        val schema = vlexNative.patternToSchema(pattern)

        // parse query and optimize it
        vlexNative.optimize(query)
        // vlexNative.parse()

        spark.stop()
    }
}