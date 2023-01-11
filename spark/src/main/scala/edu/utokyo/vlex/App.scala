package edu.utokyo.vlex

import org.apache.spark.sql.SparkSession

object App {
    def main(args: Array[String]): Unit = {
        val spark = SparkSession.builder.appName("Vlex Spark").getOrCreate()
        val queryStr: String = args(0)
        val filename: String = args(1)

        val query = Query()

        val before = System.currentTimeMillis()
        
        val queryOp = query.queryStrToQuery(spark, queryStr)
        val df = spark.read.format("edu.utokyo.vlex").schema(query.queryStrToSchema(queryStr)).options(Map("pattern" -> Query.queryStrToPattern(queryStr), "keyOption" -> Query.queryStrToKeyOp(queryStr), "query" -> Query.queryStrToQuery(queryStr))).load(filename)
        
        val startTime = System.currentTimeMillis()
        queryOp(df).show()
        val queryTime = System.currentTimeMillis() - startTime
        println("Query Time: " + queryTime / 1000.0)
        
        val timeMs = System.currentTimeMillis() - before
        println("Total Job Time: " + timeMs / 1000.0)        
        
        spark.stop()
    }
}