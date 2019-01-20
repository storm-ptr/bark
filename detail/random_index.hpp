// Andrew Naplavkov

#ifndef BARK_RANDOM_INDEX_HPP
#define BARK_RANDOM_INDEX_HPP

#include <mutex>
#include <random>

namespace bark {

class random_index {
public:
    using generator_type = std::mt19937;
    using value_type = generator_type::result_type;

    explicit random_index(value_type size)
        : gen_(std::random_device()()), dist_(0, size - 1)
    {
    }

    value_type operator()()
    {
        std::lock_guard lock{guard_};
        return dist_(gen_);
    }

private:
    std::mutex guard_;
    generator_type gen_;
    std::uniform_int_distribution<value_type> dist_;
};

}  // namespace bark

#endif  // BARK_RANDOM_INDEX_HPP
