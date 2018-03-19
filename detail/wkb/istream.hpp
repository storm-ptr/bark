// Andrew Naplavkov

#ifndef BARK_DETAIL_WKB_ISTREAM_HPP
#define BARK_DETAIL_WKB_ISTREAM_HPP

#include <bark/detail/blob_stream.hpp>
#include <bark/detail/wkb/common.hpp>
#include <cstdint>

namespace bark {
namespace detail {
namespace wkb {

class istream {
public:
    explicit istream(const uint8_t* ptr) : is_{ptr}, endian_{HostEndian} {}
    const uint8_t* data() const { return is_.data(); };

    uint8_t read_byte_order()
    {
        is_ >> endian_;
        switch (endian_) {
            case BigEndian:
            case LittleEndian:
                return endian_;
        }
        throw std::runtime_error("WKB byte order");
    }

    uint32_t read_uint32()
    {
        auto res = is_.read<uint32_t>();
        if (HostEndian != endian_)
            res = reversed(res);
        if (res != 0)
            return res;
        throw std::runtime_error("unsupported WKB value");
    }

    double read_double()
    {
        auto res = is_.read<double>();
        if (HostEndian != endian_)
            res = reversed(res);
        return res;
    }

private:
    blob_istream is_;
    uint8_t endian_;
};

}  // namespace wkb
}  // namespace detail
}  // namespace bark

#endif  // BARK_DETAIL_WKB_ISTREAM_HPP
