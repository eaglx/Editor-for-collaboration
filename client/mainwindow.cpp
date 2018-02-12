#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "connectdatamanage.h"

void MainWindow::sendMessage(int internalFlag)
{
    emit messageSent(internalFlag);
}

void MainWindow::internalMessage(int internalFlag)
{
    QTextCursor cursor = ui->textEdit->textCursor(); // Get cursor position

    int lenDataFromSerwer = dataFromServer.length();
    int curPos = cursor.position();

    disconnect(ui->textEdit, 0, this, 0);
    if(internalFlag == FLAG_UPDATE_FROM_SERV)
        ui->textEdit->setPlainText(QString::fromStdString(dataFromServer));

    if(lenDataFromSerwer < curPos)
        cursor.setPosition(lenDataFromSerwer);
    else
        cursor.setPosition(curPos);
    ui->textEdit->setTextCursor(cursor);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

void diffSearch(int len)
{
    std::string toSend = "";
    bool enterFlag = 0;
    bool exitFlag = 0;
    int enter = 0;
    int exit = 0;

    for(int i = 0; i < len; i++) {
        if(dataFromServer[i] != dataFromQTextEdit[i] && exitFlag == 0) {
           if(enterFlag == 0) {
                enter = i;
                enterFlag = 1;
           }
           toSend += dataFromQTextEdit[i];
        }
        else if(enterFlag == 1 && exitFlag == 0) {
            exitFlag = 1;
            exit = i;
        }

        //if enter and exit was made are there are some changes send it
        if(enterFlag == 1 && exitFlag == 1 && dataFromServer[i] != dataFromQTextEdit[i]) {
            //cout << "Jeszcze: " << i << " " << dataFromQTextEdit[i] << endl;
            send_to_server(FLAG_REPLACE, i, 0, dataFromQTextEdit[i], "");
        }

        //if enter was made but exit no and i is last set exit to 'i'
        if(enterFlag == 1 && exitFlag == 0 && (i == (len - 1)) ) {
            exit = i;
        }

    }
    if(enterFlag == 1) {
        int toSendLen = exit - enter;
        if(toSendLen == 1) {
            char pom = toSend[0];
            //cout << "Jeden: " << pom << " " << enter << endl;
            send_to_server(FLAG_REPLACE, enter, 0, pom, "");
        }
        else
            //cout << "Od: " << enter << ", do: " << exit  << " " << toSend << endl;
            send_to_server(FLAG_REPLACE_CHARS, enter, toSendLen, ' ', toSend);
    }
}

/* Code that executes on text change here */
void MainWindow::onTextChanged()
{
    dataFromQTextEdit = ui->textEdit->toPlainText().toUtf8().constData();

    int lenServer = dataFromServer.length();
    int lenQText = dataFromQTextEdit.length();

    //change one char
    if(lenServer == lenQText) {
        diffSearch(lenServer);
    }

    //add new chars
    if(lenQText > lenServer) {
        int lenToAdd = lenQText - lenServer;

        std::string toSend = "";

        diffSearch(lenServer);

        toSend.append(dataFromQTextEdit,lenServer,lenToAdd);

        if(lenToAdd == 1) {
            char pom = toSend[0];
            send_to_server(FLAG_APPEND, 0, 0, pom, "");
        }
        else
            send_to_server(FLAG_APPEND_CHARS, 0, lenToAdd, ' ', toSend);
    }

    //delete chars
    if(lenServer > lenQText) {
        int lenToRM = lenServer - lenQText;
        if(lenQText == 0)
        {
            send_to_server(FLAG_DEL_ALL, 0, 0, ' ', "");
        }
        else
        {
            diffSearch(lenQText);
            send_to_server(FLAG_RM, lenToRM, 0, ' ', "");
        }
    }

}

void MainWindow::getSelection()
{
    qDebug() << "onSelectionChanged:";
    QTextCursor tc = ui->textEdit->textCursor();
    qDebug() << tc.selectionStart();
    qDebug() << tc.selectionEnd();
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
