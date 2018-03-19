// Andrew Naplavkov

#ifndef BARK_DATASET_DETAIL_ISTREAM_HPP
#define BARK_DATASET_DETAIL_ISTREAM_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/detail/blob_stream.hpp>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace bark {
namespace dataset {
namespace detail {

class istream {
public:
    explicit istream(const uint8_t* ptr) : is_{ptr} {}
    explicit istream(blob_view buf) : istream{buf.data()} {}
    const uint8_t* data() const { return is_.data(); };

    variant_view read()
    {
        switch (is_.read<uint8_t>()) {
            case which<variant_view, boost::blank>():
                return boost::blank{};
            case which<variant_view, int64_view>():
                return std::cref(is_.read<int64_t>());
            case which<variant_view, double_view>():
                return std::cref(is_.read<double>());
            case which<variant_view, boost::string_view>(): {
                auto res = is_.read<boost::string_view>(is_.read<uint64_t>());
                is_.read<boost::string_view::value_type>();  // zero terminated
                return res;
            }
            case which<variant_view, blob_view>():
                return is_.read<blob_view>(is_.read<uint64_t>());
            default:
                throw std::runtime_error("invalid data type");
        }
    }

    istream& operator>>(variant_view& v)
    {
        v = read();
        return *this;
    }

    template <typename VariantViews>
    std::enable_if_t<
        std::is_same<typename VariantViews::value_type, variant_view>::value,
        istream&>
    operator>>(VariantViews& range)
    {
        for (auto& v : range)
            *this >> v;
        return *this;
    }

private:
    bark::detail::blob_istream is_;
};

}  // namespace detail
}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_DETAIL_ISTREAM_HPP
