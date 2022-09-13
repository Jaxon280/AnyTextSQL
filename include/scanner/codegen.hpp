#pragma once

#include "common.hpp"
#include "scanner/interface.hpp"

namespace vlex {

class Codegen {
    std::string code;
    int fd;
    std::string filename;
    std::vector<Qlabel> qlabels;
    std::set<ST_TYPE> states;

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

   public:
    Codegen(const std::string &filename, std::vector<Qlabel> &qlabels,
            std::set<ST_TYPE> &states);
    ~Codegen();

    int generate();
};

}  // namespace vlex
