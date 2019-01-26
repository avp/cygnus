#include "MainWindow.h"
#include <QApplication>

namespace cygnus {

class MainApp : public QApplication {
  std::vector<std::unique_ptr<MainWindow>> windows_{};

#ifdef Q_OS_MACOS
  MainWindow *firstWindow_{};
  QString fileName_;
#endif

public:
  MainApp(int argc, char *argv[]) : QApplication(argc, argv) {
#ifdef Q_OS_MACOS
    firstWindow_ = createWindow();
#else
    auto *window = createWindow();
    QApplication::processEvents();

    QString fileName{};

    if (argc > 1 && argv[1]) {
      fileName = argv[1];
    }

    if (!fileName.isEmpty()) {
      window->setFileName(fileName);
      window->loadFile();
    } else {
      window->open();
    }
#endif
  }

protected:
#ifdef Q_OS_MACOS
  bool event(QEvent *event) override {
    switch (event->type()) {
    case QEvent::FileOpen: {
      QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
      if (fileOpenEvent) {
        fileName_ = fileOpenEvent->file();
        if (!fileName_.isEmpty()) {
          MainWindow *window = firstWindow_ && !firstWindow_->isLoaded()
                                   ? firstWindow_
                                   : window = createWindow();
          window->setFileName(fileName_);
          window->loadFile();
          return true;
        }
      }
    }
    default:
      break;
    }
    return QApplication::event(event);
  }
#endif

private:
  MainWindow *createWindow() {
    windows_.emplace_back(std::make_unique<MainWindow>());
    windows_.back()->showMaximized();
    QApplication::processEvents();
    return windows_.back().get();
  }
};

} // namespace cygnus

int main(int argc, char *argv[]) {
  QCoreApplication::setOrganizationName("Cygnus Crosswords");

  cygnus::MainApp a(argc, argv);
  return a.exec();
}
