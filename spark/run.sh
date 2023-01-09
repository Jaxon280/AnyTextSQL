#!/usr/bin/env bash

if [ "$#" -lt 3 ]; then
    echo "At least five arguments required: [--local | --yarn] [query ID] [Filename]"
    exit 1
fi

MASTER=$1
SPARK_HOME=/home/natsuoiida/research/spark-2.2.0-bin-hadoop2.7

if [ $MASTER == "--local" ]; then
    MASTER=local[1]
elif [ $MASTER == "--yarn" ]; then
    MASTER=yarn
else
    echo "First argument must be either --local or --yarn"
    exit 1
fi

set -x
  # --conf "spark.dynamicAllocation.enabled=false" \
  # --num-executors 2 \

$SPARK_HOME/bin/spark-submit \
    --class edu.utokyo.vlex.App \
    --master $MASTER \
    --conf spark.sql.files.maxPartitionBytes=100000000000 \
    target/vlex-1.0.jar \
    ${@:2}