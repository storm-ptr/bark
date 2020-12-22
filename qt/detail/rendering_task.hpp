// Andrew Naplavkov

#ifndef BARK_QT_RENDERING_TASK_HPP
#define BARK_QT_RENDERING_TASK_HPP

#include <QPainter>
#include <QVector>
#include <bark/qt/detail/rendering.hpp>
#include <bark/qt/detail/start_thread.hpp>
#include <cmath>
#include <future>
#include <memory>
#include <mutex>

namespace bark::qt {

inline bool tiny(const geometry::box& tile, const geometry::box& pixel)
{
    static constexpr int MinTilePixels{128 * 128};
    return boost::geometry::area(tile) / boost::geometry::area(pixel) <
           MinTilePixels;
}

/// Asynchronous painting class. The final image is returned in the destructor.
class rendering_task : public std::enable_shared_from_this<rendering_task> {
public:
    explicit rendering_task(const georeference& ref) : ref_{ref} {}

    ~rendering_task() { promise_.set_value({ref_, img_}); }

    std::future<geoimage> get_future() { return promise_.get_future(); }

    geoimage get_recent()
    {
        auto lock = std::lock_guard{guard_};
        return {ref_, img_};
    }

    void start(QVector<layer> lrs)
    {
        static constexpr int PriorityCount{10};
        for (auto& lr : lrs)
            start_thread([self = shared_from_this(), lr = std::move(lr)] {
                self->check();
                auto pj = projection(lr);
                auto tf = proj::transformer{pj, self->ref_.projection};
                auto ext = tf.backward(extent(self->ref_));
                auto px = tf.backward(pixel(self->ref_));
                auto tls = tile_coverage(lr, ext, px);
                self->check();
                if (!tls.empty() && tiny(tls.front(), px)) {
                    auto mock_lr = lr;
                    mock_lr.brush.setStyle(Qt::NoBrush);
                    auto maps = mock_rendering(mock_lr, tls, self->ref_);
                    self->check();
                    self->compose(maps);
                }
                else
                    for (size_t i = 0; i < tls.size(); ++i) {
                        auto task = [self, lr, tl = tls[i]] {
                            self->check();
                            auto maps = rendering(lr, tl, self->ref_);
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
        auto lock = std::lock_guard{guard_};
        is_canceled_ = true;
    }

private:
    const georeference ref_;
    std::promise<geoimage> promise_;

    std::mutex guard_;
    bool is_canceled_ = false;
    QImage img_;

    void check()
    {
        auto lock = std::lock_guard{guard_};
        if (is_canceled_)
            throw cancel_exception{};
    }

    void compose(const QVector<geoimage>& maps)
    {
        for (auto& map : maps) {
            if (map.img.isNull())
                continue;
            auto lock = std::lock_guard{guard_};
            if (img_.isNull())
                img_ = make<QImage>(ref_);
            QPainter painter{&img_};
            painter.setCompositionMode(QPainter::CompositionMode_Darken);
            painter.drawImage(offset(map.ref, ref_), map.img);
        }
    }
};

}  // namespace bark::qt

#endif  // BARK_QT_RENDERING_TASK_HPP
