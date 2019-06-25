// Andrew Naplavkov

#include "insertion_task.h"
#include <QElapsedTimer>
#include <bark/proj/transformer.hpp>
#include <bark/qt/common_ops.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/sliced.hpp>

namespace {

const size_t MaxRowNumber = 1000000;
const size_t MaxVariableNumber = 999;  // @see https://sqlite.org/limits.html
const qint64 TimeoutMs = 3000;

auto column_map(const bark::qt::layer& lr)
{
    string_map res;
    for (const auto& col : attr_names(lr))
        res.insert({col, col});
    return res;
}

template <class RandomAccessRng, class Functor>
void for_each_slice(const RandomAccessRng& rng, size_t slice_size, Functor f)
{
    for (size_t pos = 0, sz = std::size(rng); pos < sz;) {
        auto prev = pos;
        pos += std::min(sz - pos, slice_size);
        f(boost::adaptors::slice(rng, prev, pos));
    }
}

class inserter {
public:
    inserter(task& tsk, bark::qt::layer lr)
        : tsk_{tsk}, lr_{std::move(lr)}, cmd_{lr_.provider->make_command()}
    {
        cmd_->set_autocommit(false);
        timer_.start();
    }

    size_t affected() const { return affected_; }

    template <class ColumnNames, class Rows>
    void operator()(const ColumnNames& cols, const Rows& rows)
    {
        cmd_->exec(insert_sql(*lr_.provider, qualifier(lr_.name), cols, rows));
        affected_ += std::size(rows);
    }

    void commit(bool force)
    {
        if (force || timer_.elapsed() >= TimeoutMs) {
            cmd_->commit();
            tsk_.push_output(QString("affected: %1").arg(affected_));
            timer_.start();
        }
    };

private:
    task& tsk_;
    bark::qt::layer lr_;
    bark::db::command_holder cmd_;
    size_t affected_ = 0;
    QElapsedTimer timer_;
};

}  // anonymous namespace

insertion_task::insertion_task(QVector<bark::qt::layer> from,
                               bark::qt::link to,
                               action act)
    : f_(([from = std::move(from), to = std::move(to), act](
              insertion_task& self) {
        if (action::PrintSqlOnly == act)
            self.push_output("-- print only --");
        self.push_output(to.uri.toDisplayString(QUrl::DecodeReserved));
        for (const auto& lr_from : from) {
            if (action::PrintSqlOnly == act)
                ::push_output(self, script(lr_from, to).second);
            else {
                auto lr_to = self.create(lr_from, to);
                emit self.refresh_sig();
                self.insert(lr_from, lr_to, column_map(lr_from));
            }
        }
    }))
{
}

insertion_task::insertion_task(bark::qt::layer from,
                               bark::qt::layer to,
                               string_map cols)
    : f_(([from = std::move(from), to = std::move(to), cols = std::move(cols)](
              insertion_task& self) {
        self.push_output(to.uri.toDisplayString(QUrl::DecodeReserved));
        self.insert(from, to, cols);
    }))
{
}

bark::qt::layer insertion_task::create(const bark::qt::layer& from,
                                       const bark::qt::link& to)
{
    auto [name, sql] = script(from, to);
    exec(*to.provider, sql);
    to.provider->refresh();
    ::push_output(*this, sql);
    bark::qt::layer_def def;
    def.name = id(name, from.name.back());
    return {to, def};
}

void insertion_task::insert(const bark::qt::layer& from,
                            const bark::qt::layer& to,
                            string_map cols)
{
    cols.insert({from.name.back(), to.name.back()});
    auto geom_pos = std::distance(cols.begin(), cols.find(from.name.back()));
    auto limit = std::max<size_t>(1, MaxVariableNumber / cols.size());
    auto tf = bark::proj::transformer{projection(from), projection(to)};
    auto insert = inserter{*this, to};
    while (true) {
        auto rowset = fetch_all(*from.provider,
                                select_sql(*from.provider,
                                           qualifier(from.name),
                                           insert.affected(),
                                           MaxRowNumber));
        auto rows = select(cols | boost::adaptors::map_keys, rowset);
        for_each_slice(rows, limit, [&](auto&& slice) {
            if (!tf.is_trivial())
                bark::db::for_each_blob(slice, geom_pos, tf.inplace_forward());
            insert(cols | boost::adaptors::map_values, slice);
            insert.commit(/*force*/ false);
        });
        if (rows.size() < MaxRowNumber)
            break;
    }
    insert.commit(/*force*/ true);
    to.provider->refresh();
}

void insertion_task::run_event()
{
    f_(*this);
}
