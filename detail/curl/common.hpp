// Andrew Naplavkov

#ifndef BARK_DETAIL_CURL_COMMON_HPP
#define BARK_DETAIL_CURL_COMMON_HPP

#include <curl/curl.h>
#include <memory>
#include <stdexcept>

namespace bark {
namespace detail {
namespace curl {

inline void check(bool res)
{
    if (!res)
        throw std::runtime_error("cURL error");
}

inline void check(CURLcode res)
{
    if (CURLE_OK == res)
        return;
    const std::string msg(curl_easy_strerror(res));
    check(!msg.empty());
    throw std::runtime_error(msg);
}

inline void check(CURLMcode res)
{
    if (CURLM_OK == res)
        return;
    const std::string msg(curl_multi_strerror(res));
    check(!msg.empty());
    throw std::runtime_error(msg);
}

struct multi_handle_deleter {
    void operator()(CURLM* p) const { curl_multi_cleanup(p); }
};

using multi_handle_holder = std::unique_ptr<CURLM, multi_handle_deleter>;

struct easy_handle_deleter {
    CURLM* multi = nullptr;

    void operator()(CURL* p) const
    {
        if (multi)
            curl_multi_remove_handle(multi, p);
        curl_easy_cleanup(p);
    }
};

using easy_handle_holder = std::unique_ptr<CURL, easy_handle_deleter>;

}  // namespace curl
}  // namespace detail
}  // namespace bark

#endif  // BARK_DETAIL_CURL_COMMON_HPP
