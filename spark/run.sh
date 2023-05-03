#!/usr/bin/env bash

# set -x
  # --conf "spark.dynamicAllocation.enabled=false" \
  # --num-executors 2 \

$SPARK_HOME/bin/spark-submit \
    --class edu.utokyo.vlex.App \
    --master local[1] \
    --conf spark.sql.files.maxPartitionBytes=100000000000 \
    --driver-memory 3G \
    target/vlex-1.0.jar 