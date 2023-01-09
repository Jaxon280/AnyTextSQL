#pragma once

#include "common.hpp"
#include "table.hpp"
#include "parser/query.hpp"

using namespace vlex;

StatementList *buildWildCard(bool isCount);
StatementList *buildStatements(StatementList *stmts, Statement *stmt);
Statement *buildStatement(OpTree *left, const char *right);

StringList *buildIdents(StringList *idents, const char *ident);

OpTree *buildOp(OpType op, OpTree *left, OpTree *right);
OpTree *buildAggFunc(AggFuncType ftype, const char *ident);
OpTree *buildVar(const char *ident);
OpTree *buildConstString(const char *svalue);
OpTree *buildSignedNumber(OpTree *opt, bool isNeg);
OpTree *buildConstInt(int ivalue);
OpTree *buildConstDouble(double dvalue);
PredTree *buildCond(CondType ctype, PredTree *left, PredTree *right);
PredTree *buildCCond(CondType ctype, PredTree *cond, OpTree *ccond);
PredTree *buildPred(OpType op, OpTree *left, OpTree *right);

OpTree *evalConstPred(OpType opt, OpTree *left, OpTree *right);
OpTree *evalConstCond(CondType ctype, OpTree *left, OpTree *right);
OpTree *evalConstExpr(OpType opt, OpTree *left, OpTree *right);
