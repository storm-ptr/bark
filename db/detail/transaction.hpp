// Andrew Naplavkov

#ifndef BARK_DB_DETAIL_TRANSACTION_HPP
#define BARK_DB_DETAIL_TRANSACTION_HPP

namespace bark {
namespace db {
namespace detail {

template <typename T>
class transaction {
    T& as_mixin() { return static_cast<T&>(*this); }

protected:
    void set_autocommit(bool autocommit)
    {
        if (autocommit_ != autocommit) {
            exec(as_mixin(), autocommit ? "ROLLBACK" : "BEGIN");
            autocommit_ = autocommit;
        }
    }

    void commit()
    {
        if (!autocommit_) {
            exec(as_mixin(), "COMMIT");
            exec(as_mixin(), "BEGIN");
        }
    }

private:
    bool autocommit_ = true;
};

}  // namespace detail
}  // namespace db
}  // namespace bark

#endif  // BARK_DB_DETAIL_TRANSACTION_HPP
