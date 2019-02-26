/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::ImageWidget class.
 * ---------------------------------------------------------------- */

#pragma once

#include <QtWidgets/QWidget>

namespace kuu
{
namespace rasperi
{

class Controller;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(
        Controller* controller,
        QWidget* parent = nullptr);
    void setImage(const QImage& image);

protected:
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
