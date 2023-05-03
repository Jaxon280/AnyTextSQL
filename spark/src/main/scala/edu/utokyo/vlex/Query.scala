package edu.utokyo.vlex

import org.apache.spark.sql.functions._
import org.apache.spark.sql.types._
import org.apache.spark.sql.{DataFrame, SparkSession}

case class Query() {
  def queryStrToQuery(spark: SparkSession, queryStr: String): (Array[DataFrame]) => DataFrame = {
    import spark.implicits._

    queryStr match {
      case "yelp0" =>
        /**
          * SELECT stars
          * FROM yelp.json
          * WHERE stars > 4.5
          */
        (df: Array[DataFrame]) => {
          df(0).filter($"stars" > 4.5)
        }
      case "yelp1" =>

        /**
          * SELECT categories, AVG(stars)
          * FROM yelp.json
          * WHERE stars > 3.5
          * GROUP BY categories;
          */
        (df: Array[DataFrame]) => {
          df(0).filter($"stars" > 3.5).select($"categories", $"stars").groupBy($"categories").agg(avg($"stars"))
        }
      case "yelp2" =>
        (df: Array[DataFrame]) => {
          // val yelp_b: DataFrame = df(0)
          // val yelp_c: DataFrame = df(1)
          // yelp_b.filter(yelp_b("b_stars") > 3.5).select(yelp_b("b_business_id"), yelp_b("b_stars")).join(yelp_c, yelp_b("b_business_id") === yelp_c("c_business_id")).groupBy(yelp_b("c_business_id"))
          df(0)
        }
      case _ =>
        (df: Array[DataFrame]) => {
          df(0)
        }
    }
  }

  def queryStrToSchema(queryStr: String): Array[StructType] = {
    val json = Metadata.fromJson("""{"length": 64}""")
    queryStr match {
      case "yelp0" =>
        Array(new StructType().add("stars", DoubleType))
      case "yelp1" =>
        Array(new StructType().add("stars", DoubleType).add("categories", StringType, nullable=false, json))
      case "yelp2" =>
        Array(new StructType().add("b_business_id", StringType, nullable=false, json).add("b_stars", DoubleType), new StructType().add("c_business_id", StringType, nullable=false, json))
      case _ =>
        // Default value for every other query
        Array(new StructType().add("value", LongType))
    }
  }
}

object Query {
  val queryStrToNumDF = Map(
    "yelp0" -> 1,
    "yelp1" -> 1,
    "yelp2" -> 2
  )
  val queryStrToFilenames = Map(
    "yelp0" -> ("yelp_b.json"),
    "yelp1" -> ("yelp_b.json"),
    "yelp2" -> ("yelp_b.json", "yelp_c.json")
  )
  val queryStrToCommand = Map(
    "yelp0" -> (".scan yelp_b.json -e '\"stars\":(?P<stars>DOUBLE)' -t yelp_b"),
    "yelp1" -> (".scan yelp_b.json -k ['\"stars\":(?P<stars>DOUBLE)', '\"categories\":\"(?P<categories>[^\"]+)\"'] -t yelp_b"),
    "yelp2" -> (".scan yelp_b.json -k ['\"business_id\":\"(?P<b_business_id>[^\"]+)\"', '\"stars\":(?P<b_stars>DOUBLE)'] -t yelp_b", ".scan yelp_c.json -e '\"business_id\":\"(?P<c_business_id>[^\"]+)\"' -t yelp_c")
  )
  val queryStrToQuery = Map(
    "yelp0" -> ("select stars from yelp where stars > 4.5;"),
    "yelp1" -> ("select categories, avg(stars) from yelp where stars > 3.5 group by categories;"),
    "yelp2" -> ("select b_business_id, b_stars from yelp_b where b_stars > 3.5 group by b_business_id;", "select c_business_id from yelp_c;")
  )
}