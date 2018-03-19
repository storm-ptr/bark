// Andrew Naplavkov

#ifndef BARK_DETAIL_EXPECTED_HPP
#define BARK_DETAIL_EXPECTED_HPP

#include <boost/variant.hpp>
#include <boost/variant/static_visitor.hpp>
#include <stdexcept>
#include <type_traits>

namespace bark {
namespace detail {

/**
 * expected<T> is either a T or the exception preventing its creation.
 * Synchronous std::future<T> analogue.
 * @see
 * http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
 */
template <typename T>
class expected {
public:
    explicit expected(const T& val) : state_(val) {}

    explicit expected(T&& val) : state_(std::move(val)) {}

    explicit expected(std::exception_ptr e)
        : state_(e ? e
                   : std::make_exception_ptr(std::logic_error("no exception")))
    {
    }

    const T& get() const
    {
        struct visitor : boost::static_visitor<const T&> {
            const T& operator()(const T& val) const { return val; }
            const T& operator()(const std::exception_ptr& e) const
            {
                std::rethrow_exception(e);
            }
        };
        return boost::apply_visitor(visitor{}, state_);
    }

private:
    boost::variant<T, std::exception_ptr> state_;
};

template <typename Functor>
auto make_expected(Functor fn) noexcept
{
    using expected_t = expected<std::decay_t<std::result_of_t<Functor()>>>;
    try {
        return expected_t(fn());
    }
    catch (const std::exception&) {
        return expected_t(std::current_exception());
    }
}

}  // namespace detail
}  // namespace bark

#endif  // BARK_DETAIL_EXPECTED_HPP
