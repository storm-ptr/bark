// Andrew Naplavkov

#include "task.h"
#include <QThreadPool>
#include <bark/dataset/rowset_ops.hpp>
#include <bark/geometry/geom_from_wkb.hpp>
#include <bark/qt/common_ops.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/uniqued.hpp>
#include <stdexcept>

task::task() : state_(status::Waiting), output_(&str_) {}

void task::run() try {
    {
        std::lock_guard<std::mutex> lock{guard_};
        switch (state_) {
            case task::status::Waiting:
                state_ = status::Running;
                break;
            case task::status::Canceling:
                throw std::runtime_error("canceled");
            default:
                return;
        }
    }

    run_event();

    std::lock_guard<std::mutex> lock{guard_};
    state_ = status::Successed;
}
catch (const std::exception& e) {
    std::lock_guard<std::mutex> lock{guard_};
    state_ = status::Failed;
    output_ << e.what() << '\n';
}

task::status task::state()
{
    std::lock_guard<std::mutex> lock{guard_};
    return state_;
}

void task::push_output(const QString& msg)
{
    auto txt = trim_right_copy(msg);

    std::lock_guard<std::mutex> lock{guard_};
    switch (state_) {
        case task::status::Running:
            if (!txt.isEmpty())
                output_ << txt << '\n';
            break;
        case task::status::Canceling:
            throw std::runtime_error("canceled");
        default:
            throw std::logic_error("task");
    }
}

QString task::pop_output()
{
    std::lock_guard<std::mutex> lock{guard_};
    auto res = trim_right_copy(str_);
    str_.clear();
    return res;
}

void task::cancel()
{
    std::lock_guard<std::mutex> lock{guard_};
    if (state_ != status::Successed && state_ != status::Failed)
        state_ = status::Canceling;
}

void push_output(task& tsk, const std::string& str)
{
    tsk.push_output(QString::fromStdString(str));
}

template <typename T>
void push_output(task& tsk, const T& val)
{
    push_output(tsk, boost::lexical_cast<std::string>(val));
}

runnable::runnable(task_ptr tsk) : tsk_{tsk} {}

void runnable::run()
{
    tsk_->run();
}

void runnable::start(task_ptr tsk)
{
    auto ptr = new runnable(tsk);
    ptr->setAutoDelete(true);
    QThreadPool::globalInstance()->start(ptr);
}

columns_task::columns_task(bark::qt::layer lhs, bark::qt::layer rhs)
    : lhs_{lhs}, rhs_{rhs}
{
}

void columns_task::run_event()
{
    emit columns_sig({lhs_, column_names(lhs_)}, {rhs_, column_names(rhs_)});
}

deletion_task::deletion_task(bark::qt::layer lr) : lr_{lr} {}

void deletion_task::run_event()
{
    push_output(lr_.uri.toDisplayString(QUrl::DecodeReserved));
    auto query = drop_sql(*lr_.provider, qualifier(lr_.name));
    ::push_output(*this, query.sql());
    exec(*lr_.provider, query);
    lr_.provider->refresh();
    emit refresh_sig();
}

sql_task::sql_task(bark::qt::link lnk, std::string sql)
    : lnk_{std::move(lnk)}, sql_{std::move(sql)}
{
}

void sql_task::run_event()
{
    push_output(lnk_.uri.toDisplayString(QUrl::DecodeReserved));
    ::push_output(*this, sql_);
    auto rows = fetch_all(*lnk_.provider, sql_);
    ::push_output(*this, rows);
}

attributes_task::attributes_task(bark::qt::layer lr, bark::qt::frame frm)
    : lr_{std::move(lr)}, frm_{std::move(frm)}
{
}

static auto make_intersects(const bark::geometry::box& ext)
{
    return [ext](auto&& row) {
        auto wkb = boost::get<bark::blob_view>(row[0]);
        auto geom = bark::geometry::geom_from_wkb(wkb.data());
        auto bbox = bark::geometry::envelope(geom);
        return boost::geometry::intersects(bbox, ext);
    };
}

void attributes_task::run_event()
{
    using namespace bark;
    static constexpr size_t Limit = 10;

    push_output(lr_.uri.toDisplayString(QUrl::DecodeReserved));
    ::push_output(*this, lr_.name);

    auto tf = proj::transformer{frm_.projection, projection(lr_)};
    auto v = tf.forward(view(frm_));
    auto intersects = make_intersects(v.extent);

    boost::optional<dataset::rowset> res;
    for (auto&& tl : tile_coverage(lr_, v)) {
        auto objects = spatial_objects(lr_, {tl, v.scale});
        auto rows = as_vector(objects);
        if (res) {
            if (res->columns() != objects.columns())
                throw std::runtime_error("columns mismatch");
            rows.insert(rows.end(), res->begin(), res->end());
        }
        std::sort(rows.begin(), rows.end());

        size_t counter = 0;
        dataset::ostream os;
        for (auto&& row : rows | boost::adaptors::uniqued |
                              boost::adaptors::filtered(intersects)) {
            os << row;
            if (++counter >= Limit)
                break;
        }
        res = dataset::rowset{objects.columns(), std::move(os).buf()};
        if (counter >= Limit)
            break;
    }
    if (res)
        ::push_output(*this, res.get());
}

metadata_task::metadata_task(bark::qt::layer lr) : lr_{std::move(lr)} {}

void metadata_task::run_event()
{
    push_output(lr_.uri.toDisplayString(QUrl::DecodeReserved));
    ::push_output(*this, lr_.name);
    auto tbl = table(lr_);
    ::push_output(*this, tbl);
}
