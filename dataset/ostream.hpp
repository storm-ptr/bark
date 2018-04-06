// Andrew Naplavkov

#ifndef BARK_DATASET_OSTREAM_HPP
#define BARK_DATASET_OSTREAM_HPP

#include <bark/dataset/variant_view.hpp>
#include <bark/detail/blob_stream.hpp>
#include <boost/variant/static_visitor.hpp>
#include <cstdint>
#include <type_traits>

namespace bark {
namespace dataset {

class ostream {
public:
    blob_view buf() const& { return os_.buf(); }
    blob_t buf() && { return std::move(os_).buf(); };

    template <typename T>
    std::enable_if_t<std::is_integral<T>::value, ostream&> operator<<(T v)
    {
        os_ << which<variant_view, int64_view>() << static_cast<int64_t>(v);
        return *this;
    }

    template <typename T>
    std::enable_if_t<std::is_floating_point<T>::value, ostream&> operator<<(T v)
    {
        os_ << which<variant_view, double_view>() << static_cast<double>(v);
        return *this;
    }

    ostream& operator<<(const variant_view& v)
    {
        boost::apply_visitor(output_visitor{os_}, v);
        return *this;
    }

    template <typename VariantViews>
    std::enable_if_t<
        std::is_same<typename VariantViews::value_type, variant_view>::value,
        ostream&>
    operator<<(const VariantViews& range)
    {
        for (const auto& v : range)
            *this << v;
        return *this;
    }

private:
    bark::detail::blob_ostream os_;

    class output_visitor : public boost::static_visitor<void> {
        bark::detail::blob_ostream& os_;

    public:
        explicit output_visitor(bark::detail::blob_ostream& os) : os_(os) {}

        void operator()(boost::blank) const
        {
            os_ << which<variant_view, boost::blank>();
        }

        template <typename T>
        void operator()(std::reference_wrapper<const T> v) const
        {
            os_ << which<variant_view, decltype(v)>() << v.get();
        }

        void operator()(string_view v) const
        {
            os_ << which<variant_view, decltype(v)>()
                << static_cast<uint64_t>(v.size()) << v
                << string_view::value_type{0};  // zero terminated
        }

        void operator()(blob_view v) const
        {
            os_ << which<variant_view, decltype(v)>()
                << static_cast<uint64_t>(v.size()) << v;
        }
    };
};

}  // namespace dataset
}  // namespace bark

#endif  // BARK_DATASET_OSTREAM_HPP
