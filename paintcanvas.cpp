#include "paintcanvas.h"
#include <queue>

PaintCanvas::PaintCanvas(QWidget* parent)
    : QFrame(parent)
{
    pixmap = QPixmap(this->width(), this->height());
    pixmap.fill();
}

void PaintCanvas::setColor(QColor c)
{
    color = c;
}

QPixmap PaintCanvas::getPixmap() const
{
    return pixmap;
}

void PaintCanvas::setFillMethod(bool v)
{
    fillMethod = v;
}

void PaintCanvas::setImagePattern(QImage ppattern)
{
    pattern = ppattern;
    patternLoaded = true;
}

void PaintCanvas::setThickness(int val)
{
    penThickness = val;
}

void PaintCanvas::clearArea()
{
    pixmap.fill();
    repaint();
}

void PaintCanvas::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
}

void PaintCanvas::mousePressEvent(QMouseEvent* e)
{
    curPos = e->pos();
    if (e->buttons() & Qt::RightButton) {
        if (!fillMethod)
            colorFill(curPos);
        else
            imageFill(curPos);
    } else
        mousePressed = true;
}

void PaintCanvas::mouseReleaseEvent(QMouseEvent*)
{
    mousePressed = false;
}

void PaintCanvas::mouseMoveEvent(QMouseEvent* e)
{
    if (!mousePressed)
        return;
    QPainter p(&pixmap);
    //p.setRenderHint(QPainter::Antialiasing);
    if (e->buttons() & Qt::LeftButton)
        p.setPen(QPen(color, penThickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.drawLine(curPos, e->pos());
    curPos = e->pos();
    repaint();
}

void PaintCanvas::resizeEvent(QResizeEvent*)
{
    pixmap = QPixmap(width(), height());
    pixmap.fill();
}

bool PaintCanvas::notBorder(QPoint p)
{
    return pixmapImg.pixelColor(p) == bgColor;
}

QVector<QPoint> getNeighborhood(const QPoint& p)
{
    QVector<QPoint> res;
    res.push_back(QPoint(p.x() + 1, p.y()));
    res.push_back(QPoint(p.x() + 1, p.y() - 1));
    res.push_back(QPoint(p.x(), p.y() - 1));
    res.push_back(QPoint(p.x() - 1, p.y() - 1));
    res.push_back(QPoint(p.x() - 1, p.y()));
    res.push_back(QPoint(p.x() - 1, p.y() + 1));
    res.push_back(QPoint(p.x(), p.y() + 1));
    res.push_back(QPoint(p.x() + 1, p.y() + 1));
    return res;
}

/*
Directions:

3   2   1
4   -   0
5   6   7
*/

QPair<QPoint, int> PaintCanvas::nextPoint(QPoint from, int dirToCheckFirst)
{
    QVector<QPoint> neighborhood = getNeighborhood(from);
    for (int i = 0; i < 8; ++i)
        if (!notBorder(neighborhood[(i + dirToCheckFirst) % 8]))
            return QPair<QPoint, int>(neighborhood[(i + dirToCheckFirst) % 8],
                (i + dirToCheckFirst + 6) % 8);
}

void PaintCanvas::deleteRedundantPoints(QQueue<QPoint>& borderPoints)
{
    int bpsize = borderPoints.size();

    for (int i = 0; i < bpsize; ++i) {
        if ((borderPoints[i].y() > borderPoints[(i + 1) % bpsize].y()) && (borderPoints[i].y() > borderPoints[(i - 1 + bpsize) % bpsize].y()))
            borderPoints.erase(borderPoints.begin() + i, borderPoints.begin() + i + 1);
        else if ((borderPoints[i].y() < borderPoints[(i + 1) % bpsize].y()) && (borderPoints[i].y() < borderPoints[(i - 1 + bpsize) % bpsize].y()))
            borderPoints.erase(borderPoints.begin() + i, borderPoints.begin() + i + 1);
        else if ((borderPoints[i].y() == borderPoints[(i + 1) % bpsize].y()) && (borderPoints[i].y() == borderPoints[(i - 1 + bpsize) % bpsize].y()))
            borderPoints.erase(borderPoints.begin() + i, borderPoints.begin() + i + 1);
    }
}

QQueue<QPair<QPoint, QPoint>> PaintCanvas::getPointsToConnect(QSet<QPoint> borderPoints)
{
    QQueue<QPair<QPoint, QPoint>> pointsToConnect;
    while (!borderPoints.empty()) {
        curPos = *borderPoints.begin();
        bool moveRight = notBorder(QPoint(curPos.x() + 1, curPos.y())) || borderPoints.contains(QPoint(curPos.x() + 1, curPos.y()));
        if (moveRight) {
            borderPoints.erase(borderPoints.begin());
        } else
            continue;
        QPoint border = getRightEnd(curPos);
        auto found = borderPoints.begin();
        if ((found = borderPoints.find(border)) != borderPoints.end()) {
            pointsToConnect.push_back(QPair<QPoint, QPoint>(curPos, border));
            borderPoints.erase(found);
        } else {
            QSet<QPoint> obstacleBorderPoints = getBorderPoints(curPos, 4);

            int sz = obstacleBorderPoints.size();
            for (int i = 0; i < sz; ++i) {
                QPoint curObstPos = *obstacleBorderPoints.begin();
                bool moveLeft = notBorder(QPoint(curObstPos.x() - 1, curObstPos.y())) || obstacleBorderPoints.contains(QPoint(curObstPos.x() - 1, curObstPos.y()));
                if (moveLeft) {
                    obstacleBorderPoints.erase(borderPoints.begin());
                } else
                    continue;
                if ((found = borderPoints.find(getLeftEnd(curObstPos))) != borderPoints.end()) {
                    pointsToConnect.push_back(QPair<QPoint, QPoint>(*found, curObstPos));
                    borderPoints.erase(found);
                } else if ((found = borderPoints.find(getRightEnd(curObstPos))) != borderPoints.end()) {
                    pointsToConnect.push_back(QPair<QPoint, QPoint>(curObstPos, *found));
                    borderPoints.erase(found);
                } else if ((found = obstacleBorderPoints.find(getLeftEnd(curObstPos))) != obstacleBorderPoints.end()) {
                    pointsToConnect.push_back(QPair<QPoint, QPoint>(*found, curObstPos));
                    obstacleBorderPoints.erase(found);
                } else if ((found = obstacleBorderPoints.find(getRightEnd(curObstPos))) != obstacleBorderPoints.end()) {
                    pointsToConnect.push_back(QPair<QPoint, QPoint>(curObstPos, *found));
                    obstacleBorderPoints.erase(found);
                }
            }
            while (!obstacleBorderPoints.empty()) {
                borderPoints.insert(*obstacleBorderPoints.begin());
                obstacleBorderPoints.erase(obstacleBorderPoints.begin());
            }
        }
    }

    return pointsToConnect;
}

QPoint PaintCanvas::getRightEnd(QPoint from)
{
    do {
        from.setX(from.x() + 1);
    } while (notBorder(from));
    return from;
}

QPoint PaintCanvas::getLeftEnd(QPoint from)
{
    do {
        from.setX(from.x() - 1);
    } while (notBorder(from));
    return from;
}

QSet<QPoint> PaintCanvas::getBorderPoints(QPoint curPos, int direction)
{
    QQueue<QPoint> res;
    res.push_back(curPos);

    QPair<QPoint, int> advance = nextPoint(curPos, direction);
    while (advance.first != curPos) {
        res.push_back(advance.first);
        advance = nextPoint(advance.first, advance.second);
    }

    deleteRedundantPoints(res);

    return res.toSet();
}

void PaintCanvas::colorFill(QPoint curPos)
{
    pixmapImg = pixmap.toImage();
    bgColor = pixmapImg.pixelColor(curPos);
    if (color == bgColor)
        return;

    while (notBorder(curPos) && (curPos.x() > 0))
        curPos.setX(curPos.x() - 1);

    QSet<QPoint> borderPoints = getBorderPoints(curPos, 0);
    QQueue<QPair<QPoint, QPoint>> pointsToConnect = getPointsToConnect(borderPoints);

    while (!pointsToConnect.empty()) {
        QPair<QPoint, QPoint> cur = pointsToConnect.takeFirst();
        pointsToConnect.pop_front();

        QPoint from = cur.first;
        do {
            pixmapImg.setPixelColor(from, color);
            from.setX(from.x() + 1);
        } while (from != cur.second);
    }

    pixmap = QPixmap::fromImage(pixmapImg);
    repaint();
}

/*void PaintCanvas::colorFill(QPoint curPos)
{
    QImage temp = pixmap.toImage();
    bgColor = temp.pixelColor(curPos);
    if (color == bgColor)
        return;

    QQueue<QPoint> pointsToFill;
    pointsToFill.push_back(curPos);

    while (!pointsToFill.empty()) {
        QPoint qTop = pointsToFill.takeFirst();
        QPoint right = qTop;
        QPoint left = qTop;
        while ((right.x() + 1 < temp.width()) && (temp.pixelColor(right) == bgColor))
            right.setX(right.x() + 1);
        while ((left.x() - 1 >= 0) && (temp.pixelColor(left.x() - 1, left.y()) == bgColor))
            left.setX(left.x() - 1);
        right.setX(qMin(right.x(), temp.width() - 1));
        left.setX(qMax(left.x(), 0));
        for (QPoint p = left; p != right; p.setX(p.x() + 1)) {
            temp.setPixelColor(p, color);
            if ((p.y() + 1 < temp.height()) && (temp.pixelColor(p.x(), p.y() + 1) == bgColor))
                pointsToFill.push_back(QPoint(p.x(), p.y() + 1));
            if ((p.y() - 1 >= 0) && (temp.pixelColor(p.x(), p.y() - 1) == bgColor))
                pointsToFill.push_back(QPoint(p.x(), p.y() - 1));
        }
    }

    pixmap = QPixmap::fromImage(temp);
    repaint();
}*/

// works, but not always
/*void PaintCanvas::imageFill(QPoint basePos)
{
    QImage temp = pixmap.toImage();
    bgColor = temp.pixelColor(basePos);

    QQueue<QPoint> pointsToFill;
    pointsToFill.push_back(basePos);

    while (!pointsToFill.empty()) {
        QPoint qTop = pointsToFill.takeFirst();
        QPoint right = qTop;
        QPoint left = qTop;
        while ((right.x() + 1 < temp.width()) && (temp.pixelColor(right) == bgColor))
            right.setX(right.x() + 1);
        while ((left.x() - 1 >= 0) && (temp.pixelColor(left.x() - 1, left.y()) == bgColor))
            left.setX(left.x() - 1);
        right.setX(qMin(right.x(), temp.width() - 1));
        left.setX(qMax(left.x(), 0));
        for (QPoint p = left; p != right; p.setX(p.x() + 1)) {
            int newX = basePos.x() - p.x();
            while (newX < 0)
                newX += pattern.width();
            int newY = basePos.y() - p.y();
            while (newY < 0)
                newY += pattern.height();
            QColor c = pattern.pixelColor(newX % pattern.width(), newY % pattern.height());
            temp.setPixelColor(p, c);
            if ((p.y() + 1 < temp.height()) && (temp.pixelColor(p.x(), p.y() + 1) == bgColor))
                pointsToFill.push_back(QPoint(p.x(), p.y() + 1));
            if ((p.y() - 1 >= 0) && (temp.pixelColor(p.x(), p.y() - 1) == bgColor))
                pointsToFill.push_back(QPoint(p.x(), p.y() - 1));
        }
    }

    pixmap = QPixmap::fromImage(temp);
    repaint();
}*/

void imageFillRecursive(QPoint basePos, QPoint curPos, const QImage& ptrn,
    QImage& tempImg, QColor bgColor)
{
    QPoint right = curPos;
    QPoint left = curPos;

    left.setX(left.x() - 1);
    while (left.x() >= 0) {
        QColor pixColor = tempImg.pixel(left);
        if (pixColor != bgColor)
            break;

        QColor c = ptrn.pixelColor((left.x() + basePos.x()) % ptrn.width(),
            (left.y() + basePos.y()) % ptrn.height());
        tempImg.setPixelColor(left, c);

        left.setX(left.x() - 1);
    }
    left.setX(left.x() + 1);

    while (right.x() < tempImg.width()) {
        QColor pixColor = tempImg.pixel(right);
        if (pixColor != bgColor)
            break;

        QColor c = ptrn.pixelColor((right.x() + basePos.x()) % ptrn.width(),
            (right.y() + basePos.y()) % ptrn.height());
        tempImg.setPixelColor(right, c);

        right.setX(right.x() + 1);
    }
    right.setX(right.x() - 1);

    for (QPoint p = left; p != right; p.setX(p.x() + 1)) {

        if ((p.y() > 0) && (tempImg.pixelColor(p.x(), p.y() - 1) == bgColor))
            imageFillRecursive(basePos, QPoint(p.x(), p.y() - 1), ptrn, tempImg, bgColor);
        if ((p.y() < tempImg.height() - 2) && (tempImg.pixelColor(p.x(), p.y() + 1) == bgColor))
            imageFillRecursive(basePos, QPoint(p.x(), p.y() + 1), ptrn, tempImg, bgColor);
    }
}

void PaintCanvas::imageFill(QPoint basePos)
{
    if (!patternLoaded) {
        QMessageBox::warning(this, "Error", "No pattern loaded");
        return;
    }
    QImage temp = pixmap.toImage();
    bgColor = temp.pixelColor(basePos);

    imageFillRecursive(basePos, basePos, pattern, temp, bgColor);

    pixmap = QPixmap::fromImage(temp);
    repaint();
}
