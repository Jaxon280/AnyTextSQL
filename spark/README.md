# AnyDB with Apache Spark

AnyDB is integrated with Apache Spark for the query execution on the parsed file. The main code is `spark/src/main/scala/edu/utokyo/vlex/App.scala` and `JNI` is used for the integration of C++ code and Java/Scala code.

## Prerequisites

- g++ (12.0.0) and above
- An Intel processor that supports AVX-512
  - rigel node in Taura laboratory
- Java 8 (note: **You cannot run Apache Spark code by using Java 11 and above**)
- JNI (1.8.0)
  - You can get Java 8 and JNI from OpenJDK[https://wiki.openjdk.org/display/jdk8u/Main]
- Apache Spark (2.2.0, Hadoop 2.7)
  - Download the library from here[https://archive.apache.org/dist/spark/spark-2.2.0/spark-2.2.0-bin-hadoop2.7.tgz]. And set the path to downloaded & decompressed directory as `$SPARK_HOME` enviroment variable. 
- Apache Maven (3.8.6)
- Flex
- Bison
- Also, you should set PATH to `g++`, `flex`, `bison`, `javah` and `mvn` commands.

## Quick Start

```Makefile
make
make run # run the pragram
```
