#include <CustomPlotItem.hxx>

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QUrl>
#include <QJSEngine>
#include <QDebug>
#include <QCoreApplication>
#include <QMouseEvent>
#include <QWheelEvent>

CustomPlotItem::CustomPlotItem(QQuickItem* parent)
    : QQuickPaintedItem(parent), m_customPlot(new QCustomPlot())
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    connect(m_customPlot, &QCustomPlot::afterReplot, this, [this]() { update(); });
}

CustomPlotItem::~CustomPlotItem()
{
    delete m_customPlot;
}

void CustomPlotItem::paint(QPainter* painter)
{
    if (m_customPlot) {
        QPixmap picture(boundingRect().size().toSize());
        picture.fill(Qt::white);
        QCPPainter qcpPainter(&picture);
        m_customPlot->toPainter(&qcpPainter);
        painter->drawPixmap(QPoint(), picture);
    }
}

void CustomPlotItem::geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    if (m_customPlot) {
        m_customPlot->setGeometry(0, 0, newGeometry.width(), newGeometry.height());
        m_customPlot->setViewport(QRect(0, 0, newGeometry.width(), newGeometry.height()));
    }
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
}

void CustomPlotItem::mousePressEvent(QMouseEvent* event) {
    QCoreApplication::sendEvent(m_customPlot, event);
    event->accept();
}

void CustomPlotItem::mouseReleaseEvent(QMouseEvent* event) {
    QCoreApplication::sendEvent(m_customPlot, event);
    event->accept();
}

void CustomPlotItem::mouseMoveEvent(QMouseEvent* event) {
    QCoreApplication::sendEvent(m_customPlot, event);
    event->accept();
}

void CustomPlotItem::mouseDoubleClickEvent(QMouseEvent* event) {
    QCoreApplication::sendEvent(m_customPlot, event);
    event->accept();
}

void CustomPlotItem::wheelEvent(QWheelEvent* event) {
    QCoreApplication::sendEvent(m_customPlot, event);
    event->accept();
}

void CustomPlotItem::resetScale()
{
    if (!m_customPlot) return;
    m_customPlot->rescaleAxes();
    m_customPlot->xAxis->scaleRange(1.05, m_customPlot->xAxis->range().center());
    m_customPlot->yAxis->scaleRange(1.05, m_customPlot->yAxis->range().center());
    m_customPlot->replot();
}

void CustomPlotItem::clearPlot()
{
    if (!m_customPlot) return;
    m_customPlot->clearGraphs();
    m_customPlot->replot();
}

void CustomPlotItem::loadData(const QString& fileUrl, const QColor& color)
{
    QString filePath = QUrl(fileUrl).toLocalFile();
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QVector<double> x, y;
    bool hasData = false;

    QVector<QColor> autoColors = {
        Qt::red, Qt::blue, Qt::green, Qt::magenta, Qt::cyan,
        Qt::darkYellow, Qt::black, Qt::darkRed, Qt::darkBlue, Qt::darkGreen
    };

    QVector<QColor> activeColors;
    activeColors.append(color);
    for (const QColor& c : autoColors) {
        if (c != color) {
            activeColors.append(c);
        }
    }

    int blockIndex = 0;

    auto commitGraph = [&]() {
        if (x.isEmpty()) return;
        QCPGraph* graph = m_customPlot->addGraph();
        graph->setData(x, y);

        graph->setLineStyle(QCPGraph::lsNone);

        QColor graphColor = activeColors[blockIndex % activeColors.size()];

        QCPScatterStyle scatterStyle(QCPScatterStyle::ssDisc, 6);
        scatterStyle.setPen(QPen(graphColor));
        scatterStyle.setBrush(QBrush(graphColor));

        graph->setScatterStyle(scatterStyle);

        blockIndex++;
        x.clear();
        y.clear();
        hasData = true;
        };

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            commitGraph();
        }
        else {
            QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                x.append(parts[0].toDouble());
                y.append(parts[1].toDouble());
            }
        }
    }
    commitGraph();

    if (hasData) {
        resetScale();
    }
}

void CustomPlotItem::plotCustomFunction(const QString& formula, double shift, double scale, double shape, const QColor& color, double xMin, double xMax, int stepCount)
{
    if (stepCount <= 1 || formula.trimmed().isEmpty()) return;

    QJSEngine engine;

    // ОБНОВЛЕНО: Подготавливаем переменные z и w заранее для удобства написания формул
    QString jsCode = QString(
        "(function(x, shift, scale, shape) { "
        "  with(Math) { "
        "    var z = abs((x - shift) / scale); "
        "    var w = sqrt(3 * shape + 1); "
        "    return %1; "
        "  } "
        "})"
    ).arg(formula);

    QJSValue jsFunc = engine.evaluate(jsCode);

    if (jsFunc.isError()) {
        qWarning() << "Formula parsing error:" << jsFunc.toString();
        return;
    }

    QVector<double> x(stepCount), y(stepCount);
    double step = (xMax - xMin) / (stepCount - 1);

    for (int i = 0; i < stepCount; ++i) {
        double currentX = xMin + i * step;
        QJSValueList args;
        args << currentX << shift << scale << shape;

        double currentY = jsFunc.call(args).toNumber();

        x[i] = currentX;
        y[i] = currentY;
    }

    QCPGraph* graph = m_customPlot->addGraph();
    graph->setData(x, y);

    graph->setLineStyle(QCPGraph::lsLine);
    graph->setScatterStyle(QCPScatterStyle::ssNone);

    QPen graphPen;
    graphPen.setColor(color);
    graphPen.setWidth(3);
    graph->setPen(graphPen);

    resetScale();
}