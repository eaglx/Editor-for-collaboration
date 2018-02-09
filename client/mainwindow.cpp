#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::sendMessage(int internalFlag)
{
    emit messageSent(internalFlag);
}

void MainWindow::internalMessage(int internalFlag)
{
    qDebug() << "internalMessage " << internalFlag << "\n";
    disconnect(ui->textEdit, 0, this, 0);
    if(internalFlag == 5)
        ui->textEdit->insertPlainText("HELLO\n");
    else if(internalFlag == 10)
        ui->textEdit->setReadOnly(true);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

void diffSearch(int len)
{
    for(int i = 0; i < len; i++) {
        if(dataFromServer[i] != dataFromQTextEdit[i]) {
            //replace(dataFromQTextEdit[i]);      //TODO
        }
    }
}

void MainWindow::onTextChanged()
{
    /* Code that executes on text change here */
    // QTextCursor  cursor = ui->textEdit->textCursor(); // Get cursor position
    dataFromQTextEdit = ui->textEdit->toPlainText().toUtf8().constData();
    qDebug() << QString::fromStdString(dataFromQTextEdit);

    int lenServer = dataFromServer.length();
    int lenQText = dataFromQTextEdit.length();

    //change one char
    if(lenServer == lenQText) {
        diffSearch(lenServer);
    }

    //add new chars
    if(lenQText > lenServer) {
        diffSearch(lenServer);

        for(int i = lenServer; i < lenQText; i++) {
            //append(dataFromQTextEdit[i]);   //TODO
        }
    }

    //delete chars
    if(lenServer > lenQText) {
        diffSearch(lenQText);
        for(int i = lenQText; i < lenServer; i++) {
             //delete_(i);            //TODO
        }
    }

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    connect(this, SIGNAL(messageSent(int)), this, SLOT(internalMessage(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
