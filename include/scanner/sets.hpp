#pragma once

#include "common/common.hpp"

namespace vlex {
struct Qstar {
    ST_TYPE source;
    ST_TYPE sink;
    std::set<ST_TYPE> states;
    std::string str;
};
}  // namespace vlex
