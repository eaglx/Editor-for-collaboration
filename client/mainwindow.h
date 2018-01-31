#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

#include <string>
extern std::string strBuffer;
extern std::ofstream logFile;
extern QString dataFromQTextEdit;
extern QString dataFromQTextEditBUFFERED;
extern volatile bool isEndProgram;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void messageSent(int);

public slots:
    void sendMessage(int);

private slots:
    void onTextChanged();
    void internalMessage(int);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
