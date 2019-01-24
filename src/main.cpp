#include "MainWindow.h"
#include <QApplication>

namespace cygnus {

class MainApp : public QApplication {
  QString fileName_{};

  MainWindow *mainWindow_;

public:
  MainApp(int argc, char *argv[])
      : QApplication(argc, argv), mainWindow_(new MainWindow()) {
    mainWindow_->showMaximized();

    if (argc > 1 && argv[1]) {
      fileName_ = argv[1];
    }

    if (!fileName_.isEmpty()) {
      mainWindow_->setFileName(fileName_);
      mainWindow_->loadFile();
    } else {
      mainWindow_->open();
    }
  }

protected:
  bool event(QEvent *event) override {
    switch (event->type()) {
    case QEvent::FileOpen: {
      QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
      if (fileOpenEvent) {
        fileName_ = fileOpenEvent->file();
        if (!fileName_.isEmpty()) {
          if (mainWindow_) {
            mainWindow_->setFileName(fileName_);
          }
          return true;
        }
      }
    }
    default:
      break;
    }
    return QApplication::event(event);
  }
};
}

int main(int argc, char *argv[]) {
  QCoreApplication::setOrganizationName("Cygnus Crosswords");

  cygnus::MainApp a(argc, argv);
  QApplication::processEvents();
  return a.exec();
}
