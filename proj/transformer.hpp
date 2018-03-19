// Andrew Naplavkov

#ifndef BARK_PROJ_TRANSFORMER_HPP
#define BARK_PROJ_TRANSFORMER_HPP

#include <bark/common.hpp>
#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/detail/projection.hpp>
#include <bark/proj/detail/stream.hpp>
#include <bark/proj/detail/transform.hpp>

namespace bark {
namespace proj {

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

    uint8_t* inplace_forward(uint8_t* wkb) const
    {
        return detail::stream{pj_from_, pj_to_}(wkb);
    }

    uint8_t* inplace_forward(blob_t& wkb) const
    {
        return inplace_forward(const_cast<uint8_t*>(wkb.data()));
    }

    uint8_t* inplace_backward(uint8_t* wkb) const
    {
        return detail::stream{pj_to_, pj_from_}(wkb);
    }

    uint8_t* inplace_backward(blob_t& wkb) const
    {
        return inplace_backward(const_cast<uint8_t*>(wkb.data()));
    }

private:
    detail::projection pj_from_;
    detail::projection pj_to_;
};

}  // namespace proj
}  // namespace bark

#endif  // BARK_PROJ_TRANSFORMER_HPP
