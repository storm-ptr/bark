// Andrew Naplavkov

#ifndef BARK_PROJ_TRANSFORMER_HPP
#define BARK_PROJ_TRANSFORMER_HPP

#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/detail/projection.hpp>
#include <bark/proj/detail/stream.hpp>
#include <bark/proj/detail/transform.hpp>

namespace bark::proj {

class transformer {
public:
    transformer(const std::string& pj_from, const std::string& pj_to)
        : pj_from_(pj_from), pj_to_(pj_to)
    {
    }

    bool is_trivial() const { return pj_from_.get_def() == pj_to_.get_def(); }

    geometry::point forward(const geometry::point& point) const
    {
        return detail::transformed(pj_from_, pj_to_, point);
    }

    geometry::point backward(const geometry::point& point) const
    {
        return detail::transformed(pj_to_, pj_from_, point);
    }

    geometry::box forward(const geometry::box& box) const
    {
        return detail::transformed(pj_from_, pj_to_, box);
    }

    geometry::box backward(const geometry::box& box) const
    {
        return detail::transformed(pj_to_, pj_from_, box);
    }

    geometry::view forward(const geometry::view& view) const
    {
        auto ext = forward(view.extent);
        auto px = forward(geometry::pixel(view));
        return {ext, geometry::max_scale(px)};
    }

    geometry::view backward(const geometry::view& view) const
    {
        auto ext = backward(view.extent);
        auto px = backward(geometry::pixel(view));
        return {ext, geometry::max_scale(px)};
    }

    void inplace_forward(double* begin, double* end) const
    {
        detail::transform(pj_from_, pj_to_, begin, end);
    }

    void inplace_backward(double* begin, double* end) const
    {
        detail::transform(pj_to_, pj_from_, begin, end);
    }

    void inplace_forward(blob_view wkb) const
    {
        detail::stream{pj_from_, pj_to_}(wkb);
    }

    void inplace_backward(blob_view wkb) const
    {
        detail::stream{pj_to_, pj_from_}(wkb);
    }

    auto inplace_forward() const
    {
        return [this](auto... args) { inplace_forward(args...); };
    }

    auto inplace_backward() const
    {
        return [this](auto... args) { inplace_backward(args...); };
    }

private:
    detail::projection pj_from_;
    detail::projection pj_to_;
};

}  // namespace bark::proj

#endif  // BARK_PROJ_TRANSFORMER_HPP
