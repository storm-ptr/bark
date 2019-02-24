// Andrew Naplavkov

#ifndef BARK_EXPECTED_HPP
#define BARK_EXPECTED_HPP

#include <bark/utility.hpp>
#include <stdexcept>

namespace bark {

/**
 * expected<T> is either a T or the exception preventing its creation.
 * Synchronous std::future<T> analogue.
 * @see
 * http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
 */
template <class T>
class expected {
public:
    explicit expected(T val) : state_(std::move(val)) {}

    explicit expected(std::exception_ptr e)
        : state_(e ? e
                   : std::make_exception_ptr(std::logic_error("no exception")))
    {
    }

    const T& get() const
    {
        return std::visit(
            overloaded{[](const auto& val) -> const T& { return val; },
                       [](const std::exception_ptr& e) -> const T& {
                           std::rethrow_exception(e);
                       }},
            state_);
    }

private:
    std::variant<T, std::exception_ptr> state_;
};

template <class Functor>
auto make_expected(Functor f) noexcept
{
    using expected_t = expected<std::decay_t<decltype(f())>>;
    try {
        return expected_t(f());
    }
    catch (const std::exception&) {
        return expected_t(std::current_exception());
    }
}

}  // namespace bark

#endif  // BARK_EXPECTED_HPP
