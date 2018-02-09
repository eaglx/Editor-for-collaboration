#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

#include <string>

#define FLAG_UPDATE_FROM_SERV 111

extern std::ofstream logFile;
extern std::string dataFromQTextEdit;
extern std::string dataFromServer;
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
