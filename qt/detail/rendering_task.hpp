// Andrew Naplavkov

#ifndef BARK_QT_DETAIL_RENDERING_TASK_HPP
#define BARK_QT_DETAIL_RENDERING_TASK_HPP

#include <QPainter>
#include <QVector>
#include <bark/qt/detail/rendering.hpp>
#include <bark/qt/detail/start_thread.hpp>
#include <cmath>
#include <future>
#include <memory>
#include <mutex>

namespace bark::qt::detail {

inline bool tiny(const geometry::box& tile, const geometry::view& view)
{
    static constexpr int MinTilePixels{128 * 128};
    return boost::geometry::area(tile) / boost::geometry::area(pixel(view)) <
           MinTilePixels;
}

/// Asynchronous painting class. The final image is returned in the destructor.
class rendering_task : public std::enable_shared_from_this<rendering_task> {
public:
    explicit rendering_task(const frame& frm) : frm_{frm} {}

    ~rendering_task() { promise_.set_value({frm_, img_}); }

    std::future<canvas> get_future() { return promise_.get_future(); }

    canvas get_recent()
    {
        std::lock_guard lock{guard_};
        return {frm_, img_};
    }

    void start(QVector<layer> lrs)
    {
        static constexpr int PriorityCount{10};
        for (auto& lr : lrs)
            start_thread([self = shared_from_this(), lr = std::move(lr)] {
                self->check();
                auto tf =
                    proj::transformer{self->frm_.projection, projection(lr)};
                auto v = tf.forward(view(self->frm_));
                auto tls = tile_coverage(lr, v);
                self->check();
                if (!tls.empty() && tiny(tls.front(), v)) {
                    auto mock_lr = lr;
                    mock_lr.brush.setStyle(Qt::NoBrush);
                    auto maps = mock_rendering(mock_lr, tls, self->frm_);
                    self->check();
                    self->compose(maps);
                }
                else
                    for (size_t i = 0; i < tls.size(); ++i) {
                        auto task = [self, lr, tl = tls[i]] {
                            self->check();
                            auto maps = rendering(lr, tl, self->frm_);
                            self->check();
                            self->compose(maps);
                        };
                        auto priority = PriorityNormal -
                                        PriorityCount * i / double(tls.size());
                        start_thread(task, round(priority));
                    }
            });
    }

    void cancel()
    {
        std::lock_guard lock{guard_};
        is_canceled_ = true;
    }

private:
    const frame frm_;
    std::promise<canvas> promise_;

    std::mutex guard_;
    bool is_canceled_ = false;
    QImage img_;

    void check()
    {
        std::lock_guard lock{guard_};
        if (is_canceled_)
            throw cancel_exception{};
    }

    void compose(const QVector<canvas>& maps)
    {
        for (auto& map : maps) {
            if (map.img.isNull())
                continue;
            std::lock_guard lock{guard_};
            if (img_.isNull())
                img_ = make<QImage>(frm_);
            QPainter painter{&img_};
            painter.setCompositionMode(QPainter::CompositionMode_Darken);
            painter.drawImage(offset(map.frm, frm_), map.img);
        }
    }
};

}  // namespace bark::qt::detail

#endif  // BARK_QT_DETAIL_RENDERING_TASK_HPP
