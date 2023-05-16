#pragma once

// We no longer support codegen because VFA execution system is swtiched to runtime-base approach.

#include "common/common.hpp"
#include "scanner/vfa.hpp"

namespace vlex {

class Codegen {
   public:
    Codegen(const std::string &filename, std::vector<Qlabel> &qlabels,
            std::set<ST_TYPE> &states);
    ~Codegen();
    int generate();

   private:
    void add_preprocess();
    void add_tokenClass();
    void add_member();
    void add_cmpstr(Delta *d);
    void add_constructor();
    void add_generate_token();
    void add_lex();
    void add_inv();
    void add_intrisic(Qlabel &label, std::string q);
    void add_trans(Qlabel &label, std::string q);
    void add_vfaClass();

    std::string code;
    int fd;
    std::string filename;
    std::vector<Qlabel> qlabels;
    std::set<ST_TYPE> states;
};

}  // namespace vlex
