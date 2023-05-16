package edu.utokyo.vlex

import org.apache.spark.sql.{DataFrame, SparkSession}

import org.apache.spark.sql.functions._
import org.apache.spark.sql.types._

object App {
    def main(args: Array[String]): Unit = {
        val spark = SparkSession.builder.appName("Vlex Spark").getOrCreate()
        import spark.implicits._
        val query = Query()        
        val startTime = System.currentTimeMillis()
        // val df = spark.read.format("json").load{"../sample.json"}
        // val df1 = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("b_business_id", StringType, nullable=false, json).add("b_stars", DoubleType)).options(Map("command" -> ".scan yelp_b.json -k ['\"business_id\":\"(?P<b_business_id>[^\"]+)\"', '\"stars\":(?P<b_stars>DOUBLE)'] -t yelp_b", "query" ->"select b_business_id, b_stars from yelp_b where b_stars > 3.5 group by b_business_id;")).load("yelp_b.json")
        // val df2 = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("c_business_id", StringType, nullable=false, json)).options(Map("command" -> ".scan yelp_c.json -e '\"business_id\":\"(?P<c_business_id>[^\"]+)\"' -t yelp_c", "query" -> "select c_business_id from yelp_c;")).load("yelp_c.json")
        // val df = df1.filter(df1("b_stars") > 3.5).select(df1("b_business_id"), df1("b_stars")).join(df2, df1("b_business_id") === df2("c_business_id")).groupBy(df1("b_business_id")).agg(avg(df1("b_stars")))
        
        // df1.filter(df1("b_stars") > 3.5).select(df1("b_business_id"), df1("b_stars")).join(df2, df1("b_business_id") === df2("c_business_id")).groupBy(df1("b_business_id")).agg(avg(df1("b_stars"))).show()
        // val df1 = spark.read.format("json").load("yelp_b.json")
        // val df2 = spark.read.format("json").load("yelp_c.json")
        // val df = df1.filter(df1("stars") > 3.5).select(df1("business_id"), df1("stars")).join(df2, df1("business_id") === df2("business_id")).groupBy(df1("business_id")).agg(avg(df1("stars")))
        // val df = spark.read.format("json").load("yelp_b.json").filter($"stars" > 3.5).groupBy($"categories").agg(avg($"stars")).limit(20)
        
        // Jackson / Yelp / average stars by categories
        // val df = spark.read.format("json").load("yelp_b.json")
        // spark.time(spark.read.format("json").load("yelp_b.json").filter($"categories".contains("Restaurants")).select($"categories", $"stars").groupBy($"categories").agg(avg($"stars")).show(truncate=false))

        // Vlex / Yelp / average stars by categories
        // val df = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("stars", DoubleType).add("categories", StringType, nullable=false, json2)).options(Map("command" -> ".scan yelp_b.json -k ['\"stars\":(?P<stars>DOUBLE)', '\"categories\":\"(?P<categories>[^\"]+)\"'] -t yelp_b", "query" ->"select categories, stars from yelp_b where categories LIKE '%Restaurants%' group by categories;")).load("yelp_b.json")
        // spark.time(df.filter($"categories".contains("Restaurants")).select($"categories", $"stars").groupBy($"categories").agg(avg($"stars")).show(truncate=false))
        
        // Jackson / Twitter / sum retweets by author
        // val df = spark.read.format("json").option("multiline", true).load("tweet_34GB.json")
        // println(df.filter($"text".contains("Donald Trump") && $"created_at".contains("2023")).count())
        // spark.time(df.filter($"text".contains("Obama")).select($"author_id", $"retweet_count").groupBy($"author_id").agg(sum($"retweet_count")).show(truncate=false))
     
        // xSIG Twitter
        // val df = spark.read.format("edu.utokyo.vlex").options(Map("command" -> ".scan tweet.json -k [ '\"id\": \"[^\"]*\"', '\"text\": \"[^\"]*football[^\"]*\"'] -t tweet", "query" ->"select * from tweet;")).load("tweet.json")
        // println(df.count())
        // val df = spark.read.format("edu.utokyo.vlex").options(Map("command" -> ".scan tweet.json -e '\"text\": \"[^\"]*Trump[^\"]*\"' -t tweet", "query" ->"select * from tweet;")).load("tweet.json")
        // println(df.count())

        // xSIG Yelp JSON
        // val df = spark.read.format("edu.utokyo.vlex").options(Map("command" -> ".scan yelp_b.json -k [ '\"business_id\":\"[^\"]+\"', '\"categories\":\"[^\"]*Restaurants[^\"]*\"'] -t yelp_b", "query" ->"select * from yelp_b;")).load("yelp_b.json")
        // println(df.count())
        // val df = spark.read.format("edu.utokyo.vlex").options(Map("command" -> ".scan yelp_b.json -e '\"stars\":DOUBLE' -t yelp", "query" ->"select * from yelp;")).load("yelp_b.json")
        // println(df.count())

        // xSIG Yelp CSV
        // val df = spark.read.format("edu.utokyo.vlex").options(Map("command" -> ".scan review.csv -e '\\n5,[^,]*,[^,]*,\"[^\"]*lol[^\"]*\",[^,]*,DOUBLE,[^\\n]*' -t usac", "query" ->"select * from usac;")).load("review.csv")
        // println(df.count())

        // val df = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("city", StringType, nullable=false, json1)).options(Map("command" -> ".scan US_Accidents_Dec21_updated.csv -e '\\n[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,[^,]*,(?P<city>[^,]*),' -t usac", "query" ->"select city from usac;")).load("US_Accidents_Dec21_updated.csv")
        // println(df.filter($"city".contains("Cleveland")).count())
        query.vlexTwitter3(spark)
        
        val totalTime = System.currentTimeMillis() - startTime
        println("Job Time: " + totalTime / 1000.0)
        
        spark.stop()
    }
}