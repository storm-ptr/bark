// Andrew Naplavkov

#ifndef BARK_CURL_HPP
#define BARK_CURL_HPP

#include <bark/blob.hpp>
#include <curl/curl.h>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace bark {

template <class Key>
class curl {
public:
    using value_type = std::pair<Key, std::unique_ptr<blob>>;

    bool empty() const { return jobs_.empty(); }
    size_t size() const { return jobs_.size(); }

    explicit curl(std::string useragent) : useragent_{std::move(useragent)}
    {
        static std::once_flag flag;
        std::call_once(flag, curl_global_init, CURL_GLOBAL_ALL);
        multi_.reset(curl_multi_init());
        check(!!multi_);
    }

    void push(const Key& key, const std::string& url)
    {
        using namespace std::chrono;
        static long TimeoutMs = duration_cast<milliseconds>(DbTimeout).count();
        auto easy = curl_easy_init();
        auto holder = easy_holder{easy};
        check(!!holder);
        auto buf = std::make_unique<blob>();
        check(curl_easy_setopt(easy, CURLOPT_USERAGENT, useragent_.c_str()));
        check(curl_easy_setopt(easy, CURLOPT_URL, url.c_str()));
        check(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &callback));
        check(curl_easy_setopt(easy, CURLOPT_WRITEDATA, buf.get()));
        check(curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, TimeoutMs));
        check(curl_multi_add_handle(multi_.get(), easy));
        holder.get_deleter().multi = multi_.get();
        jobs_[easy] = {std::move(holder), value_type{key, std::move(buf)}};
    }

    value_type pop()
    {
        while (true) {
            int running = 0;
            while (CURLM_CALL_MULTI_PERFORM ==
                   curl_multi_perform(multi_.get(), &running))
                ;
            if (running == int(jobs_.size()))
                continue;
            CURLMsg* msg = nullptr;
            do {
                int queue = 0;
                msg = curl_multi_info_read(multi_.get(), &queue);
                if (msg && msg->msg == CURLMSG_DONE) {
                    auto pos = jobs_.find(msg->easy_handle);
                    if (pos != jobs_.end())
                        return std::move(jobs_.extract(pos).mapped().second);
                }
            } while (msg);
        }
    }

private:
    struct multi_deleter {
        void operator()(CURLM* p) const { curl_multi_cleanup(p); }
    };

    struct easy_deleter {
        CURLM* multi = nullptr;

        void operator()(CURL* p) const
        {
            curl_multi_remove_handle(multi, p);
            curl_easy_cleanup(p);
        }
    };

    using multi_holder = std::unique_ptr<CURLM, multi_deleter>;
    using easy_holder = std::unique_ptr<CURL, easy_deleter>;
    using job_context = std::pair<easy_holder, value_type>;

    std::string useragent_;
    multi_holder multi_;
    std::unordered_map<CURL*, job_context> jobs_;

    static void check(bool res)
    {
        if (!res)
            throw std::runtime_error("cURL: unknown error");
    }

    static void check(CURLcode res)
    {
        using namespace std::string_literals;
        if (CURLE_OK != res)
            throw std::runtime_error("cURL: "s + curl_easy_strerror(res));
    }

    static void check(CURLMcode res)
    {
        using namespace std::string_literals;
        if (CURLM_OK != res)
            throw std::runtime_error("cURL: "s + curl_multi_strerror(res));
    }

    static size_t callback(void* ptr, size_t size, size_t nmemb, blob* buf)
    {
        write((const std::byte*)ptr, size * nmemb, *buf);
        return nmemb;
    }
};

}  // namespace bark

#endif  // BARK_CURL_HPP
