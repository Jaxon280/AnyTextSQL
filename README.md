# AnyDB

This code base implements AnyDB, parser and query executor for the raw data. AnyDB can be applied to **any** format by specifying the pattern of the extraction in regular expression faster than the exisiting parsers. Also, you can execute SQL-like query in AnyDB on the raw data for analytics tasks. Note that this repository is the beta-version of AnyDB.

## Integration with Apache Spark

The code in a root directory is for CLI execution. We also support execution with Apache Spark in `/spark` directory. If you want to run AnyDB with Apache Spark, go to `/spark` directory and follow the instructions in `/spark/README.md`.

## Quick Start
```console
make
./anydb
```

Then give the raw data and extraciton pattern like following one.
```
>>> .scan /path/to/large/file.json -t data -e "stars":(?P<stars>DOUBLE).*"categories":"(?P<categories>[a-zA-Z]*)"
```

You can execute the query on the specified pattern.
```
>>> select categories, avg(stars) from data where stars > 3.5 group by categories limit 30;
```
