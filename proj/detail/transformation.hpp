// Andrew Naplavkov

#ifndef BARK_PROJ_TRANSFORMATION_HPP
#define BARK_PROJ_TRANSFORMATION_HPP

#include <bark/proj/detail/utility.hpp>
#include <stdexcept>
#include <string>

namespace bark::proj {

class transformation {
public:
    explicit transformation(const std::string& from, const std::string& to)
        : ctx_{proj_context_create()}
        , tf_{ctx_ ? proj_create_crs_to_crs(ctx_.get(),
                                            from.c_str(),
                                            to.c_str(),
                                            nullptr)
                   : nullptr}
        , for_gis_{tf_ ? proj_normalize_for_visualization(ctx_.get(), tf_.get())
                       : nullptr}
    {
        if (!for_gis_)
            throw_error();
    }

    void trans_generic(PJ_DIRECTION dir, double* first, double* last) const
    {
        size_t count = (size_t)std::distance(first, last) / 2;
        size_t res = proj_trans_generic(for_gis_.get(),
                                        dir,
                                        first,
                                        2 * sizeof(double),
                                        count,
                                        first + 1,
                                        2 * sizeof(double),
                                        count,
                                        nullptr,
                                        0,
                                        0,
                                        nullptr,
                                        0,
                                        0);
        if (count != res)
            throw_error();
    }

private:
    ctx_holder ctx_;
    pj_holder tf_;
    pj_holder for_gis_;

    void throw_error() const
    {
        std::string msg;
        if (ctx_)
            msg = proj_errno_string(proj_context_errno(ctx_.get()));
        if (msg.empty())
            msg = "transformation";
        throw std::runtime_error(msg.c_str());
    }
};

}  // namespace bark::proj

#endif  // BARK_PROJ_TRANSFORMATION_HPP
