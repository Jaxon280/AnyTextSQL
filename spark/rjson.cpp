#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include <iostream>
#include <sstream>
#include <cstdio>
#include <fcntl.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
 
using namespace rapidjson;

int main() {
    const int gb = (1 << 31) - 1;
    char *buf = new char[gb];

    // FILE* fd = fopen("tweet_3GB.json", "rb");
    // FileReadStream is(fd, buf, gb);

    int fd = open("yelp_b.json", O_RDONLY);
    int sz = read(fd, buf, gb);

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    // if JSON:

    // Document doc;
    // doc.Parse(buf);
    // // doc.ParseStream(is);
 
    // // Check for parse errors
    // if (doc.HasParseError()) {
    //     std::cerr << "Error parsing JSON: "
    //               << doc.GetParseError() << std::endl;
    //     return 1;
    // }
 
    // // Iterate over the array of objects
    // Value::ConstValueIterator itr;
    // std::string keyword("football");
    // int count = 0;
 
    // for (itr = doc.Begin(); itr != doc.End(); ++itr) {
    //     // Access the data in the object
    //     // if (itr->GetObject()["public_metrics"].GetObject()["retweet_count"].GetInt() >= 0) {
    //     //     count++;
    //     // }
    //     std::string key(itr->GetObject()["text"].GetString());
    //     if (itr->GetObject()["id"].GetString() != "" && key.find(keyword) != std::string::npos) {
    //         count++;
    //     }
    // }
    // std::cout << count << std::endl;

    // if JSONLines:
    std::stringstream bufs;
    bufs << buf;
    std::string record;
    int count = 0;
    std::string keyword("Persian");
    while(std::getline(bufs, record)) {  
        Document doc;
        doc.Parse(record.c_str());    
        if (doc.HasParseError()) { 
            std::cerr << "Error parsing JSON: "
                    << doc.GetParseError() << std::endl;
            break;
        }
        if (doc.GetObject()["categories"].IsString()) {
            std::string key(doc.GetObject()["categories"].GetString());
            if (key.find(keyword) != std::string::npos) {
                count++;
            }
        }
    }   
    std::cout << count << std::endl;

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_parse: %lf s\n\n", ex_time);
#endif

    // fclose(fd);
    close(fd);
    return 0;
}