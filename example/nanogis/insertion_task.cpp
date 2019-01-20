// Andrew Naplavkov

#include "insertion_task.h"
#include <QElapsedTimer>
#include <bark/qt/common_ops.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/sliced.hpp>

namespace {

const size_t MaxRowNumber = 1000000;
const size_t MaxVariableNumber = 999;  // @see https://sqlite.org/limits.html
const qint64 TimeoutMs = 3000;

auto column_map(bark::qt::layer lr)
{
    string_map res;
    for (auto& col : attr_names(lr))
        res.insert({col, col});
    return res;
}

template <class RandomAccessRng, class Functor>
void for_each_slice(RandomAccessRng&& rng, size_t slice_size, Functor f)
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
        : tsk_{tsk}, lr_{lr}, cmd_{lr_.provider->make_command()}
    {
        cmd_->set_autocommit(false);
        timer_.start();
    }

    size_t affected() const { return affected_; }

    template <class ColumnNames, class Rows>
    void operator()(const ColumnNames& cols, const Rows& rows)
    {
        exec(*cmd_, insert_sql(*lr_.provider, qualifier(lr_.name), cols, rows));
        affected_ += boost::size(rows);
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
    : fun_(([=](insertion_task* self) {
        if (action::PrintSqlOnly == act)
            self->push_output("-- print only --");
        self->push_output(to.uri.toDisplayString(QUrl::DecodeReserved));
        for (auto& lr_from : from) {
            if (action::PrintSqlOnly == act)
                ::push_output(*self, script(lr_from, to).sql);
            else {
                auto lr_to = self->create(lr_from, to);
                emit self->refresh_sig();
                self->insert(lr_from, lr_to, column_map(lr_from));
            }
        }
    }))
{
}

insertion_task::insertion_task(bark::qt::layer from,
                               bark::qt::layer to,
                               string_map cols)
    : fun_(([=](insertion_task* self) {
        self->push_output(to.uri.toDisplayString(QUrl::DecodeReserved));
        self->insert(from, to, cols);
    }))
{
}

bark::qt::layer insertion_task::create(bark::qt::layer from, bark::qt::link to)
{
    auto scr = script(from, to);
    exec(*to.provider, scr.sql);
    to.provider->refresh();
    ::push_output(*this, scr.sql);
    bark::qt::layer_def def;
    def.name = id(scr.name, from.name.back());
    return {to, def};
}

void insertion_task::insert(bark::qt::layer from,
                            bark::qt::layer to,
                            string_map cols)
{
    cols.insert({from.name.back(), to.name.back()});
    auto geom_pos = std::distance(cols.begin(), cols.find(from.name.back()));
    auto limit = std::max<size_t>(1, MaxVariableNumber / cols.size());
    auto tf = bark::proj::transformer{projection(from), projection(to)};
    auto insert = inserter{*this, to};
    while (true) {
        auto rows = select(*from.provider,
                           qualifier(from.name),
                           cols | boost::adaptors::map_keys,
                           insert.affected(),
                           MaxRowNumber);
        auto rng = range(rows);
        for_each_slice(rng, limit, [&](auto&& slice) {
            if (!tf.is_trivial())
                bark::db::for_each_blob(slice, geom_pos, tf.inplace_forward());
            insert(cols | boost::adaptors::map_values, slice);
            insert.commit(false);
        });
        if (rng.size() < MaxRowNumber)
            break;
    }
    insert.commit(true);
    to.provider->refresh();
}

void insertion_task::run_event()
{
    fun_(this);
}
