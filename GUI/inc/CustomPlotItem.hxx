#ifndef GUI_CustomPlotItem_Header
#define GUI_CustomPlotItem_Header

#include <QtQuick/QQuickPaintedItem>
#include "qcustomplot.h"
#include <QColor>

class CustomPlotItem : public QQuickPaintedItem
{
    Q_OBJECT
public:
    explicit CustomPlotItem(QQuickItem* parent = nullptr);
    virtual ~CustomPlotItem();

    void paint(QPainter* painter) override;

    Q_INVOKABLE void loadData(const QString& fileUrl, const QColor& color);
    Q_INVOKABLE void plotCustomFunction(const QString& formula, double v, double a, double b, double mu, double sigma, const QColor& color, double xMin = -5.0, double xMax = 5.0, int stepCount = 500);

    Q_INVOKABLE void resetScale();
    Q_INVOKABLE void clearPlot();

protected:
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QCustomPlot* m_customPlot;
};

#endif
