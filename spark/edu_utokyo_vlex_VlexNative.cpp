#include "edu_utokyo_vlex_VlexNative.h"
#include <jni.h>

#include "command.hpp"
#include "common.hpp"
#include "parser/command.hpp"
#include "spark.hpp"

JNIEXPORT jlong JNICALL Java_edu_utokyo_vlex_VlexNative_parse
  (JNIEnv *env, jobject, jstring filename_java, jint filename_length, jobject buffer, jint sizeInRow, jint varSize, jstring pattern_java, jint pattern_length, jboolean isKeys, jstring query_java, jint query_length) {
    std::string filename_c(env->GetStringUTFChars(filename_java, NULL), filename_length);
    std::string pattern_c(env->GetStringUTFChars(pattern_java, NULL), pattern_length);
    std::string query_c(env->GetStringUTFChars(query_java, NULL), query_length);

    std::string cmdStr = ".scan ";
    cmdStr += "yelp.json"; // TODO: fix this
    if (isKeys) {
        cmdStr += " -k ";
    } else {
        cmdStr += " -e ";
    }
    cmdStr += "'";
    cmdStr += pattern_c;
    cmdStr += "'";

    cmdStr += " -t ";
    cmdStr += "yelp"; // TODO: fix this

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    CommandExecutor *cmd = new CommandExecutor();
    CommandContext *cmdCtx = cmd->parseCommand(cmdStr);
    cmd->execScan(cmdCtx);

    void* buffer_addr = env->GetDirectBufferAddress(buffer);
    SparkContext *sctx = new SparkContext(buffer_addr, sizeInRow, varSize);
    cmd->execParseWithSpark(query_c, sctx);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_parse: %lf s\n\n", ex_time);
#endif

    return sctx->count;
}