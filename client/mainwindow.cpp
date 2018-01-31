#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::onTextChanged()
{
    /* Code that executes on text change here */
    // QTextCursor  cursor = ui->textEdit->textCursor(); // Get cursor position
    QString data = ui->textEdit->toPlainText();

    qDebug() << data;

    // disconnect(ui->textEdit, 0, this, 0);
    // connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
