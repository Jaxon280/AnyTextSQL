package edu.utokyo.vlex

import org.apache.spark.sql.functions._
import org.apache.spark.sql.types._
import org.apache.spark.sql.{DataFrame, SparkSession}


case class Query() {
  val json = Metadata.fromJson("""{"length": 64}""")

  def vlexTwitter1(spark: SparkSession): Unit = {
    import spark.implicits._

    val df = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("created_at", StringType, nullable=false, json).add("text", StringType, nullable=false, json)).options(Map("command" -> ".scan tweet_34GB.json -k ['\"created_at\": \"(?P<created_at>[^\"]+)\"', '\"text\": \"(?P<text>[^\"]+)\"'] -t tweet", "query" ->"select text from tweet where text LIKE '%Donald Trump%' AND created_at LIKE '%2023-02%';")).load("tweet_34GB.json")
    spark.time(df.filter($"text".contains("Donald Trump") && $"created_at".contains("2023-02")).count())
  }

  def vlexTwitter2(spark: SparkSession): Unit = {
    import spark.implicits._

    val df = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("author_id", StringType, nullable=false, json).add("retweet_count", LongType).add("text", StringType, nullable=false, json)).options(Map("command" -> ".scan tweet_34GB.json -k ['\"author_id\": \"(?P<author_id>[^\"]+)\"', '\"retweet_count\": (?P<retweet_count>INT)', '\"text\": \"(?P<text>[^\"]+)\"'] -t tweet", "query" ->"select author_id, SUM(retweet_count) from tweet where text LIKE '%Obama%' group by author_id;")).load("tweet_34GB.json")
    spark.time(df.filter($"text".contains("Obama")).select($"author_id", $"retweet_count").groupBy($"author_id").agg(sum($"retweet_count")).show(truncate=false))
  }

  def vlexTwitter3(spark: SparkSession): Unit = {
    import spark.implicits._

    val df = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("lang", StringType, nullable=false, json)).options(Map("command" -> ".scan tweet_34GB.json -e '\"lang\": \"(?P<lang>[^\"]+)\"' -t tweet", "query" ->"select lang from tweet where lang == 'msa';")).load("tweet_34GB.json")
    spark.time(df.filter($"lang".contains("msa")).count())
  }

  def vlexTwitter4(spark: SparkSession): Unit = {
    import spark.implicits._

    val df = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("author_id", StringType, nullable=false, json).add("text", StringType, nullable=false, json)).options(Map("command" -> ".scan tweet_34GB.json -k ['\"author_id\": \"(?P<author_id>[^\"]+)\"', '\"text\": \"(?P<text>[^\"]+)\"'] -t tweet", "query" ->"select author_id from tweet where text LIKE '%@realDonaldTrump%';")).load("tweet_34GB.json")
    spark.time(df.filter($"text".contains("@realDonaldTrump")).select($"author_id").distinct.show(truncate=false))
  }

  // def vlexYelp1(spark: SparkSession): Unit = {
  //   import spark.implicits._

  //   val df_u = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("b_business_id", StringType, nullable=false, json).add("b_stars", DoubleType)).options(Map("command" -> ".scan yelp_b.json -k ['\"business_id\":\"(?P<b_business_id>[^\"]+)\"', '\"stars\":(?P<b_stars>DOUBLE)'] -t yelp_b", "query" ->"select b_business_id, b_stars from yelp_b where b_stars > 3.5 group by b_business_id;")).load("yelp_b.json")
  //   val df_r = spark.read.format("edu.utokyo.vlex").schema(new StructType().add("c_business_id", StringType, nullable=false, json)).options(Map("command" -> ".scan yelp_c.json -e '\"business_id\":\"(?P<c_business_id>[^\"]+)\"' -t yelp_c", "query" -> "select c_business_id from yelp_c;")).load("yelp_c.json")
  //   val df = df1.filter(df1("b_stars") > 3.5).select(df1("b_business_id"), df1("b_stars")).join(df2, df1("b_business_id") === df2("c_business_id")).groupBy(df1("b_business_id")).agg(avg(df1("b_stars")))
  //   // spark.time(df.)
  // }
}