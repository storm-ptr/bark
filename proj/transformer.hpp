// Andrew Naplavkov

#ifndef BARK_PROJ_TRANSFORMER_HPP
#define BARK_PROJ_TRANSFORMER_HPP

#include <bark/geometry/geometry_ops.hpp>
#include <bark/proj/detail/projection.hpp>
#include <bark/proj/detail/stream.hpp>
#include <bark/proj/detail/transform.hpp>

namespace bark::proj {

/// Performs conversions between cartographic projections.

/// PROJ.4 library wrapper.
/// @see https://en.wikipedia.org/wiki/PROJ
class transformer {
public:
    /// @param pj_from is a source PROJ.4 string;
    /// @param pj_to is a target PROJ.4 string.
    transformer(const std::string& pj_from, const std::string& pj_to)
        : from_(pj_from), to_(pj_to)
    {
    }

    bool is_trivial() const { return from_.get_def() == to_.get_def(); }

    geometry::point forward(const geometry::point& val) const
    {
        return transformed(from_, to_, val);
    }

    geometry::point backward(const geometry::point& val) const
    {
        return transformed(to_, from_, val);
    }

    geometry::box forward(const geometry::box& val) const
    {
        return transformed(from_, to_, val);
    }

    geometry::box backward(const geometry::box& val) const
    {
        return transformed(to_, from_, val);
    }

    void inplace_forward(double* first, double* last) const
    {
        transform(from_, to_, first, last);
    }

    void inplace_backward(double* first, double* last) const
    {
        transform(to_, from_, first, last);
    }

    void inplace_forward(blob_view wkb) const { stream{from_, to_}(wkb); }

    void inplace_backward(blob_view wkb) const { stream{to_, from_}(wkb); }

    auto inplace_forward() const
    {
        return [this](auto... args) { inplace_forward(args...); };
    }

    auto inplace_backward() const
    {
        return [this](auto... args) { inplace_backward(args...); };
    }

private:
    projection from_;
    projection to_;
};

}  // namespace bark::proj

#endif  // BARK_PROJ_TRANSFORMER_HPP
