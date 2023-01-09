package edu.utokyo.vlex

import org.apache.spark.sql.types._
import org.apache.spark.sql.{DataFrame, SparkSession}

case class Query() {
  def queryStrToQuery(spark: SparkSession, queryStr: String): (DataFrame) => Long = {
    import spark.implicits._

    queryStr match {
      case "yelp0" =>
        /**
          * SELECT COUNT()
          * FROM yelp.json
          * WHERE stars > 4.5
          */
        (df: DataFrame) => {
          df.filter($"stars" > 4.5).count()
        }
      case "yelp1" =>

        /**
          * SELECT categories, AVG(stars)
          * FROM yelp.json
          * WHERE stars > 3.5
          * GROUP BY categories;
          */
        (df: DataFrame) => {
          df.count()
        }
      case _ =>
        (df: DataFrame) => {
          df.count()
        }
    }
  }

  def queryStrToSchema(queryStr: String): StructType = {
    queryStr match {
      case "yelp0" =>
        new StructType().add("stars", DoubleType)
      case "yelp1" =>

        /**
          * SELECT autonomous_system.asn, count(ipint) AS count
          * FROM ipv4.20160425
          * WHERE autonomous_system.name CONTAINS 'Verizon'
          * GROUP BY autonomous_system.asn;
          */
        new StructType().add("stars", DoubleType).add("categories", StringType)
      case _ =>
        // Default value for every other query
        new StructType().add("value", LongType)
    }
  }
}

object Query {
  // val queryStrToKeyOp = Map(
  //   "yelp0" -> "-e",
  //   "yelp1" -> "-k")
  // val queryStrToPattern = Map(
  //   "yelp0" -> "\"stars\":(?P<stars>DOUBLE)",
  //   "yelp1" -> "\"\"")
}