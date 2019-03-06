/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   Definition of kuu::rasperi::LandingDialog class.
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>
#include <QtWidgets/QDialog>

namespace kuu
{
namespace rasperi
{

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class LandingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LandingDialog(QWidget* parent = nullptr);

signals:
    void viewPbrSphere();
    void importPhong();
    void importPbr();
    void exit();

private slots:
    void on_commandLinkButtonViewPbrSpheres_clicked();
    void on_commandLinkButtonImportPhong_clicked();
    void on_commandLinkButtonImportPbr_clicked();
    void on_commandLinkButtonExit_clicked();

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
