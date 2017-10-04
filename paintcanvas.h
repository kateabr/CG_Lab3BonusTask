#ifndef PAINTCANVAS_H
#define PAINTCANVAS_H

#include <QtWidgets>

class PaintCanvas : public QFrame {
  Q_OBJECT

public:
  PaintCanvas(QWidget *parent = nullptr);
  void setColor(QColor c);
  QPixmap getPixmap() const;
  void setFillMethod(bool v);
  void setImagePattern(QImage ppattern);

public slots:
  void setThickness(int);
  void clearArea();

protected:
  void paintEvent(QPaintEvent *);
  void mousePressEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *);
  void resizeEvent(QResizeEvent *);

private:
  QColor color = Qt::black;
  QColor bgColor = Qt::white;
  bool mousePressed = false;
  QPixmap pixmap;
  QImage pixmapImg;
  QPoint curPos;
  int penThickness = 2;
  void colorFill(QPoint curPos);
  void imageFill(QPoint basePos);
  bool fillMethod = false; // false = color, true = image
  QImage pattern;
  bool patternLoaded;
  bool notBorder(QPoint p);
  QPair<QPoint, int> nextPoint(QPoint from, int dirToCheckFirst);
  QVector<QPoint> getBorderPoints(QPoint cur, int direction);
  void deleteRedundantPoints(QVector<QPoint> &borderPoints);
  QQueue<QPair<QPoint, QPoint>>
  getPointsToConnect(QVector<QPoint> &borderPoints);
  QPoint getRightEnd(QPoint from);
  QPoint getLeftEnd(QPoint from);
  QPoint skipSpaceRight(QPoint p);
  QPoint skipSpaceLeft(QPoint p);
  bool notNeededPoint(QPoint first, QPoint second);
};

#endif // PAINTCANVAS_H
