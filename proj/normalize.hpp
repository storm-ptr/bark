// Andrew Naplavkov

#ifndef BARK_PROJ_NORMALIZE_HPP
#define BARK_PROJ_NORMALIZE_HPP

#include <bark/proj/detail/utility.hpp>
#include <boost/algorithm/string.hpp>

namespace bark::proj {

inline std::string normalize(std::string pj)
{
    auto impl = [](std::string pj) -> std::string {
        ctx_holder ctx{proj_context_create()};
        if (!ctx)
            return pj;
        proj_log_func(ctx.get(), nullptr, [](void*, int, const char*) {});
        pj_holder tf{proj_create(ctx.get(), pj.c_str())};
        if (!tf)
            return pj;
        return proj_pj_info(tf.get()).definition;
    };
    return boost::trim_copy(impl(std::move(pj)));
}

}  // namespace bark::proj

#endif  // BARK_PROJ_NORMALIZE_HPP
