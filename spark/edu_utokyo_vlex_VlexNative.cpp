#include "edu_utokyo_vlex_VlexNative.h"
#include <jni.h>

#include "command.hpp"
#include "common.hpp"
#include "parser/command.hpp"
#include "spark.hpp"

JNIEXPORT jlong JNICALL Java_edu_utokyo_vlex_VlexNative_parse
  (JNIEnv *env, jobject, jobject buffer, jint sizeInRow, jint colSize, jint varSize, jstring command_java, jint command_length, jstring query_java, jint query_length) {
    std::string command_c(env->GetStringUTFChars(command_java, NULL), command_length);
    std::string query_c(env->GetStringUTFChars(query_java, NULL), query_length);

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    CommandExecutor *cmd = new CommandExecutor();
    CommandContext *cmdCtx = cmd->parseCommand(command_c);
    cmd->execScan(cmdCtx);

    void* buffer_addr = env->GetDirectBufferAddress(buffer);
    SparkContext *sctx = new SparkContext(buffer_addr, sizeInRow, colSize, varSize);
    cmd->execWithSpark(query_c, sctx);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_parse: %lf s\n\n", ex_time);
#endif

    return sctx->count;
}