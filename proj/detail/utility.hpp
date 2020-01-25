// Andrew Naplavkov

#ifndef BARK_PROJ_UTILITY_HPP
#define BARK_PROJ_UTILITY_HPP

#include <memory>
#include <proj.h>

namespace bark::proj {

struct ctx_deleter {
    void operator()(PJ_CONTEXT* p) const { proj_context_destroy(p); }
};

using ctx_holder = std::unique_ptr<PJ_CONTEXT, ctx_deleter>;

struct pj_deleter {
    void operator()(PJ* p) const { proj_destroy(p); }
};

using pj_holder = std::unique_ptr<PJ, pj_deleter>;

}  // namespace bark::proj

#endif  // BARK_PROJ_UTILITY_HPP
