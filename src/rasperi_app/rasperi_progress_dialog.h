/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::ProgressDialog class.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>
#include <QtWidgets/QProgressDialog>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget* parent = nullptr);

public slots:
    void setProgress(int min, int value, int max);

signals:
    void doSetProgress(int min, int value, int max);

private slots:
    void onDoSetProgress(int min, int value, int max);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
