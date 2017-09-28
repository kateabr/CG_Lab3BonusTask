#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->clearButton, &QPushButton::clicked, ui->frame, &PaintCanvas::clearArea);
    connect(ui->fillColor, &QRadioButton::clicked, [&]() { ui->frame->setFillMethod(false); });
    connect(ui->fillImage, &QRadioButton::clicked, [&]() { ui->frame->setFillMethod(true); });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_colorButton_clicked()
{
    QColor temp = QColorDialog::getColor(selectedColor, this);
    if (temp.isValid()) {
        selectedColor = temp;
        ui->frame->setColor(temp);
    }
}

void MainWindow::on_loadButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load Image", "./", "*.jpg");
    if (!filename.isEmpty()) {
        QImageReader reader(filename);
        reader.setAutoTransform(true);
        QImage pattern = reader.read().convertToFormat(QImage::Format_ARGB32);
        ui->frame->setImagePattern(pattern);
    }
}
