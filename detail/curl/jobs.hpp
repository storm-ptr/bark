// Andrew Naplavkov

#ifndef BARK_DETAIL_CURL_JOBS_HPP
#define BARK_DETAIL_CURL_JOBS_HPP

#include <bark/common.hpp>
#include <bark/detail/curl/common.hpp>
#include <boost/noncopyable.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

namespace bark {
namespace detail {
namespace curl {

template <typename Key>
class jobs : private boost::noncopyable {
public:
    jobs()
    {
        static std::once_flag once_flag;
        std::call_once(once_flag, [] { curl_global_init(CURL_GLOBAL_ALL); });
        multi_.reset(curl_multi_init());
        check(!!multi_);
    }

    bool empty() const { return jobs_.empty(); }

    size_t size() const { return jobs_.size(); }

    void push(const Key& key, const std::string& url)
    {
        using namespace std::chrono;

        constexpr auto TimeoutMs =
            long(duration_cast<milliseconds>(DbTimeout).count());

        auto easy = curl_easy_init();
        job tmp{
            key, std::make_unique<blob_ostream>(), easy_handle_holder{easy}};
        check(!!easy);
        check(curl_easy_setopt(easy, CURLOPT_URL, url.c_str()));
        check(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &callback));
        check(curl_easy_setopt(easy, CURLOPT_WRITEDATA, tmp.os.get()));
        check(curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, TimeoutMs));
        check(curl_multi_add_handle(multi_.get(), easy));
        tmp.easy.get_deleter().multi = multi_.get();
        jobs_.emplace(easy, std::move(tmp));
    }

    std::pair<Key, blob_t> pop()
    {
        while (true) {
            int running(0);
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
                    if (pos != jobs_.end()) {
                        auto res = std::make_pair<Key, blob_t>(
                            std::move(pos->second.key),
                            std::move(*pos->second.os).buf());
                        jobs_.erase(pos);
                        return res;
                    }
                }
            } while (msg);
        }
    }

private:
    struct job {
        Key key;
        std::unique_ptr<blob_ostream> os;
        easy_handle_holder easy;
    };

    multi_handle_holder multi_;
    std::unordered_map<CURL*, job> jobs_;

    static size_t callback(void* ptr,
                           size_t size,
                           size_t nmemb,
                           blob_ostream* os)
    {
        *os << blob_view{(const uint8_t*)ptr, size * nmemb};
        return nmemb;
    }
};

}  // namespace curl
}  // namespace detail

namespace curl = detail::curl;

}  // namespace bark

#endif  // BARK_DETAIL_CURL_JOBS_HPP
