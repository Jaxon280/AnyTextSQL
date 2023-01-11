package edu.utokyo.vlex

import org.apache.spark.sql.functions._
import org.apache.spark.sql.types._
import org.apache.spark.sql.{DataFrame, SparkSession}

case class Query() {
  def queryStrToQuery(spark: SparkSession, queryStr: String): (DataFrame) => DataFrame = {
    import spark.implicits._

    queryStr match {
      case "yelp0" =>
        /**
          * SELECT stars
          * FROM yelp.json
          * WHERE stars > 4.5
          */
        (df: DataFrame) => {
          df.filter($"stars" > 4.5)
        }
      case "yelp1" =>

        /**
          * SELECT categories, AVG(stars)
          * FROM yelp.json
          * WHERE stars > 3.5
          * GROUP BY categories;
          */
        (df: DataFrame) => {
          df.filter($"stars" > 3.5).select($"categories", $"stars").groupBy($"categories").agg(avg($"stars"))
        }
      case _ =>
        (df: DataFrame) => {
          df
        }
    }
  }

  def queryStrToSchema(queryStr: String): StructType = {
    queryStr match {
      case "yelp0" =>
        new StructType().add("stars", DoubleType)
      case "yelp1" =>
        new StructType().add("stars", DoubleType).add("categories", StringType, nullable=false, Metadata.fromJson("""{"length": 64}"""))
      case _ =>
        // Default value for every other query
        new StructType().add("value", LongType)
    }
  }
}

object Query {
  val queryStrToQuery = Map(
    "yelp0" -> "select stars from yelp where stars > 4.5;",
    "yelp1" -> "select categories, avg(stars) from yelp where stars > 3.5 group by categories;"
  )
  val queryStrToKeyOp = Map(
    "yelp0" -> "-e",
    "yelp1" -> "-k")
  val queryStrToPattern = Map(
    "yelp0" -> "'\"stars\":(?P<stars>DOUBLE)'",
    "yelp1" -> "['\"stars\":(?P<stars>DOUBLE)', '\"categories\":\"(?P<categories>[^\"]+)\"']")
}