/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::rasperi::MainWindow class.
 * ---------------------------------------------------------------- */

#include <memory>
#include <QtWidgets/QMainWindow>

namespace Ui { class MainWindow; }

namespace kuu
{
namespace rasperi
{

class Controller;

/* ---------------------------------------------------------------- *
 * ---------------------------------------------------------------- */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        Controller* controller,
        QWidget* parent = nullptr);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace rasperi
} // namespace kuu
