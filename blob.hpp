// Andrew Naplavkov

#ifndef BARK_BLOB_HPP
#define BARK_BLOB_HPP

#include <bark/utility.hpp>
#include <iomanip>
#include <sstream>
#include <vector>

namespace bark {

/// Binary Large OBject (BLOB) - contiguous byte storage
struct blob_view : std::basic_string_view<std::byte> {
    using std::basic_string_view<std::byte>::basic_string_view;
    blob_view(const std::byte*) = delete;
};

struct blob : std::vector<std::byte> {
    using std::vector<std::byte>::vector;
    operator blob_view() const noexcept { return {data(), size()}; }
};

template <class T>
if_arithmetic_t<T, const T*> read(blob_view& src, size_t count)
{
    auto res = reinterpret_cast<const T*>(src.data());
    src.remove_prefix(count * sizeof(T));
    return res;
}

template <class T>
if_arithmetic_t<T, T> read(blob_view& src)
{
    return *read<T>(src, 1);
}

template <class T, class Size = typename T::size_type>
T read(blob_view& src)
{
    Size count = *read<Size>(src, 1);
    return {read<typename T::value_type>(src, count), count};
}

template <class T>
if_arithmetic_t<T> write(const T* src, size_t count, blob& dest)
{
    auto first = reinterpret_cast<const std::byte*>(src);
    auto last = first + count * sizeof(T);
    dest.insert(dest.end(), first, last);
}

template <class T>
if_arithmetic_t<T> write(const T& src, blob& dest)
{
    write(&src, 1, dest);
}

template <class T, class Size = typename T::size_type>
void write(const T& src, blob& dest)
{
    Size count = src.size();
    write(&count, 1, dest);
    write(src.data(), count, dest);
}

template <class T>
blob_view& operator>>(blob_view& src, T& dest)
{
    dest = read<T>(src);
    return src;
}

template <class T>
blob& operator<<(blob& dest, const T& src)
{
    write(src, dest);
    return dest;
}

struct hex {
    blob_view data;

    friend std::ostream& operator<<(std::ostream& dest, const hex& src)
    {
        std::ostringstream os;
        os << std::uppercase << std::hex << std::setfill('0');
        for (auto byte : src.data)
            os << std::setw(2) << static_cast<unsigned>(byte);
        return dest << os.str();  // call stream once
    }
};

}  // namespace bark

#endif  // BARK_BLOB_HPP
