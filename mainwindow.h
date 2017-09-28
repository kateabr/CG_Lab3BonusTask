#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private slots:
    void on_colorButton_clicked();

    void on_loadButton_clicked();

private:
    Ui::MainWindow* ui;
    QColor selectedColor = Qt::white;
};

#endif // MAINWINDOW_H
