// Andrew Naplavkov

#ifndef BARK_PROJ_PROJECTION_HPP
#define BARK_PROJ_PROJECTION_HPP

#include <memory>
#include <proj_api.h>
#include <stdexcept>
#include <string>

namespace bark::proj {

struct ctx_deleter {
    void operator()(void* p) const { pj_ctx_free(p); }
};

using ctx_holder = std::unique_ptr<void, ctx_deleter>;

struct pj_deleter {
    void operator()(void* p) const { pj_free(p); }
};

using pj_holder = std::unique_ptr<void, pj_deleter>;

class projection {
public:
    explicit projection(const std::string& def)
        : ctx_{pj_ctx_alloc()}
        , pj_{ctx_ ? pj_init_plus_ctx(ctx_.get(), def.c_str()) : nullptr}
    {
        if (!pj_)
            throw std::runtime_error(error());
    }

    operator projPJ() const { return pj_.get(); }

    std::string get_def() const { return pj_get_def(pj_.get(), 0); }

private:
    ctx_holder ctx_;
    pj_holder pj_;

    std::string error() const
    {
        std::string msg;
        if (ctx_)
            msg = pj_strerrno(pj_ctx_get_errno(ctx_.get()));
        if (msg.empty())
            msg = "invalid projection";
        return msg;
    }
};

}  // namespace bark::proj

#endif  // BARK_PROJ_PROJECTION_HPP
