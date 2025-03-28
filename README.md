# AnyTextSQL

This codebase implements AnyDB, a parser and query executor for raw data. AnyDB can be applied to **any** format by specifying extraction patterns using regular expressions, performing faster than existing parsers. It allows you to execute SQL-like queries directly on raw data for analytics tasks. Note that this repository is a beta version of AnyDB.

## Integration with Apache Spark

The code in the root directory is for CLI execution. We also support execution with Apache Spark in the `/spark` directory. If you want to run AnyDB with Apache Spark, navigate to the `/spark` directory and follow the instructions in `/spark/README.md`.

## Prerequisites (CLI execution)

- g++ (12.0.0) or above
- An Intel processor supporting AVX-512
  - Available on the `rigel` node in Taura laboratory
- Flex
- Bison
- Additionally, ensure that `g++`, `flex`, `bison`, `javah`, and `mvn` commands are in your PATH.

## Directory Structure

```
 |-include (C++ header files)
 |-src (C++ code files)
 |-generator (Grammar files for Flex & Bison for parsing CLI commands, regexes, and queries)
 |-spark (Implementations of AnyDB with Apache Spark)
 |-example
 |-dataset
 |-tools
```

## Quick Start

```console
make
./vlex
```

Then provide raw data and extraction patterns as shown below:

```
>>> .scan /path/to/large/file.json -t data -e "stars":(?P<stars>DOUBLE).*"categories":"(?P<categories>[a-zA-Z]*)"
```

You can execute queries on the specified pattern:

```
>>> select categories, avg(stars) from data where stars > 3.5 group by categories limit 30;
```
