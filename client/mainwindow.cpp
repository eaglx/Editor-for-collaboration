#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::onTextChanged()
{
    // Code that executes on text change here
    disconnect(ui->textEdit, 0, this, 0);
    ui->textEdit->setText("Hello World");
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
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
