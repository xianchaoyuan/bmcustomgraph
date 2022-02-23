#include "CustomGraph.h"
#include <iostream>

#define MaskHeight 0.2

CustomGraph::CustomGraph(QWidget *parent) : QWidget(parent)
  , m_customPlot(new QCustomPlot(this))
{
    initUi();

    //关联选点信号
    connect(m_customPlot, &QCustomPlot::plottableDoubleClick, this, &CustomGraph::onPlotClick);
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &CustomGraph::mouseMove);
    connect(m_customPlot, &QCustomPlot::mousePress, this, &CustomGraph::mousePress);
    connect(m_customPlot, &QCustomPlot::mouseRelease, this, &CustomGraph::mouseRelease);
    connect(m_customPlot, &QCustomPlot::mouseDoubleClick, this, &CustomGraph::mouseDoubleClick);

    //图形与图例同步
    connect(m_customPlot, &QCustomPlot::selectionChangedByUser, this, &CustomGraph::selectionChanged);
}

CustomGraph::~CustomGraph()
{
    delete m_customPlot;
}

void CustomGraph::initUi()
{
//    auto *curveSelectionFrame = new QFrame(this);
    m_curveSelectionLayout = new QVBoxLayout();
    m_curveSelectionLayout->setContentsMargins(9, 6, 9, 6);

    m_customPlot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);
    m_customPlot->legend->setSelectableParts(QCPLegend::spItems);
    m_customPlot->setContextMenuPolicy(Qt::CustomContextMenu);

    // 设置游标
    m_tracer = new QCPItemTracer(m_customPlot);
    m_tracer->setInterpolating(true);
    m_tracer->setStyle(QCPItemTracer::tsCrosshair);
    m_tracer->setPen(QPen(Qt::black));
    m_tracer->setBrush(Qt::black);
    m_tracer->setSize(8);
    m_tracer->setVisible(false);

    // 游标说明s
    m_tracerLabel = new QCPItemText(m_customPlot);
    m_tracerLabel->setPen(QPen(Qt::black));
    m_tracerLabel->setPositionAlignment(Qt::AlignRight | Qt::AlignBottom);
    // 自动跟随
    m_tracerLabel->position->setParentAnchor(m_tracer->position);
    m_tracerLabel->setVisible(false);

    //BmTODO 暂时不使用
//    m_scatterGraph = m_customPlot->addGraph();
//    m_scatterGraph->setName("Scatter Graph");
//    m_scatterGraph->setLineStyle(QCPGraph::lsNone);
//    m_scatterGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssPlusCircle, Qt::red, 8));

    auto *layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addLayout(m_curveSelectionLayout);
    layout->addWidget(m_customPlot);

    // 创建上下文菜单
    m_addMarkerAction = new QAction(tr("Add Marker"), this);
    connect(m_addMarkerAction, &QAction::triggered, this, &CustomGraph::addMarker);

    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction(m_addMarkerAction);
}

void CustomGraph::addGoal(double xStart, double yStart, double xEnd, double yEnd, Goal::Condition cnd, const QColor &color)
{
    Goal goal;
    goal.line = m_customPlot->addGraph();
    goal.line->setPen(QPen(color));
    goal.line->setSelectable(QCP::stNone);
    m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    goal.lineTop = m_customPlot->addGraph();
    goal.lineTop->setPen(QPen(Qt::transparent));
    goal.lineTop->setSelectable(QCP::stNone);
    goal.lineTop->setBrush(QBrush(color, Qt::FDiagPattern));
    m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    goal.lineBottom = m_customPlot->addGraph();
    goal.lineBottom->setPen(QPen(Qt::transparent));
    goal.lineBottom->setSelectable(QCP::stNone);
    goal.lineBottom->setBrush(QBrush(color, Qt::BDiagPattern));
    m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    goal.lineTop->setChannelFillGraph(goal.line);
    goal.lineBottom->setChannelFillGraph(goal.line);
    goal.cnd = cnd;
    goal.start = QPointF(xStart, yStart);
    goal.stop = QPointF(xEnd, yEnd);

    if (goal.line != nullptr)
    {
        QVector<double> x{ xStart ,xEnd }, y{ yStart, yEnd };
        goal.line->setData(x, y);
        goal.line->setVisible(true);
    }
    if (goal.lineTop != nullptr)
    {
        QVector<double> x{ xStart ,xEnd }, y{ yStart+MaskHeight, yEnd+MaskHeight };
        goal.lineTop->setData(x, y);
        goal.lineTop->setVisible(true);
    }
    if (goal.lineBottom != nullptr)
    {
        QVector<double> x{ xStart ,xEnd }, y{ yStart-MaskHeight, yEnd-MaskHeight };
        goal.lineBottom->setData(x, y);
        goal.lineBottom->setVisible(true);
    }

    switch (cnd)
    {
        case Goal::NoCnd:
            if (goal.lineTop != nullptr)
            {
                goal.lineTop->setVisible(false);
            }
            if (goal.lineBottom != nullptr)
            {
                goal.lineBottom->setVisible(false);
            }
            break;
        case Goal::GrantAndEqual:
        case Goal::Maximum:
            if (goal.lineTop != nullptr)
            {
                goal.lineTop->setVisible(false);
            }
            break;
        case Goal::Equal:
            break;
        case Goal::LessAndEqual:
        case Goal::Minimun:
            if (goal.lineBottom != nullptr)
            {
                goal.lineBottom->setVisible(false);
            }
            break;
        default:
            break;
    }
    m_customPlot->replot();
}

void CustomGraph::plot(const QVector<QVector<double>>& data, const QStringList& legendNames)
{
    if(!m_isHoldOn)
    {
        m_customPlot->clearGraphs();
        m_customPlot->clearPlottables();
    }

    int pointNum = data.first().size();
    double xMin = m_customPlot->axisRect()->rangeDragAxis(Qt::Horizontal)->range().lower;
    double size = m_customPlot->axisRect()->rangeDragAxis(Qt::Horizontal)->range().size();
    double kx = size/pointNum;

    QVector<QPair<QVector<double>, QVector<double>>> points;
    for (const auto& vec : data)
    {
        double x = xMin;
        QVector<double> xData;
        QVector<double> yData;
        for (const auto& v : vec)
        {
            xData.push_back(x += kx);
            yData.push_back(v);
        }
        points.push_back(QPair<QVector<double>, QVector<double>>(xData, yData));
    }

    int num = points.size();
    // 紫 蓝 绿 黄 橙 粉
    static QList<QColor> color{QColor(170, 0, 255), QColor(0, 0, 255),
                               QColor(0, 255, 0),   QColor(255, 170, 0),
                               QColor(255, 0, 170), QColor(255, 255, 0)};
    while (num >= color.size())
    {
        // 默认蓝色
        color << QColor(170, 0, 255);
    }

    holdOn();
    for(int i=0; i<num; i++)
    {
        plot(points[i].first, points[i].second, QPen(color.at(i)), legendNames.at(i));
        m_customPlot->graph()->setData(points[i].first, points[i].second);
    }
    holdOff();
    m_customPlot->replot();
}

void CustomGraph::plot(const QVector<double>& x, const QVector<double>& y)
{
    if(!m_isHoldOn)
    {
        m_customPlot->clearGraphs();
        m_customPlot->clearPlottables();
    }

    m_customPlot->addGraph();
    m_customPlot->graph()->setData(x, y);

    m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    m_customPlot->replot();
}

void CustomGraph::plot(const QVector<double>& x, const QVector<double>& y, const QPen& pen, const QString &legendName)
{
    if(!m_isHoldOn)
    {
        m_customPlot->clearGraphs();
        m_customPlot->clearPlottables();
    }
    m_customPlot->addGraph();
    m_customPlot->graph()->setData(x, y);
    m_customPlot->graph()->setPen(pen);
    if(legendName != "")
        m_customPlot->graph()->setName(legendName);
    else
        m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    legendOn();
    m_customPlot->replot();
}

void CustomGraph::plot(const QVector<double>& x, const QVector<double>& y, const QPen& pen, const QCPScatterStyle &style, const QString &legendName)
{
    if(!m_isHoldOn)
    {
        m_customPlot->clearGraphs();
        m_customPlot->clearPlottables();
    }
    m_customPlot->addGraph();
    m_customPlot->graph()->setData(x, y);
    m_customPlot->graph()->setPen(pen);
    if(legendName != "")
        m_customPlot->graph()->setName(legendName);
    else
        m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    m_customPlot->graph()->setScatterStyle(style);
    legendOn();

    m_customPlot->replot();
}

void CustomGraph::plotScatters(const QVector<double>& x, const QVector<double>& y, const QCPScatterStyle &style, const QString &legendName)
{
    if(!m_isHoldOn)
    {
        m_customPlot->clearGraphs();
        m_customPlot->clearPlottables();
    }
    m_customPlot->addGraph();
    m_customPlot->graph()->setData(x, y);
    if(legendName != "")
        m_customPlot->graph()->setName(legendName);
    else
        m_customPlot->legend->removeItem(m_customPlot->legend->itemCount()-1);

    m_customPlot->graph()->setLineStyle(QCPGraph::lsNone);
    m_customPlot->graph()->setScatterStyle(style);
    legendOn();

    m_customPlot->replot();
}

void CustomGraph::setAxisRangeX(double lower, double upper)
{
    m_customPlot->axisRect()->rangeDragAxis(Qt::Horizontal)->setRange(lower,upper);
}

void CustomGraph::setAxisRangeY(double lower, double upper)
{
    m_customPlot->axisRect()->rangeDragAxis(Qt::Vertical)->setRange(lower,upper);
}

void CustomGraph::setAxisLabelX(const QString &str)
{
    m_customPlot->xAxis->setLabel(str);
}

void CustomGraph::setAxisLabelY(const QString &str)
{
    m_customPlot->yAxis->setLabel(str);
}

void CustomGraph::setAxisLabelX(const QString &str, const QFont &font, const QColor &color)
{
    m_customPlot->xAxis->setLabel(str);
    m_customPlot->xAxis->setLabelFont(font);
    m_customPlot->xAxis->setLabelColor(color);
}

void CustomGraph::setAxisLabelY(const QString &str, const QFont &font, const QColor &color)
{
    m_customPlot->yAxis->setLabel(str);
    m_customPlot->yAxis->setLabelFont(font);
    m_customPlot->yAxis->setLabelColor(color);
}

void CustomGraph::setAxisLabelFontX( const QFont &font, const QColor &color)
{
    m_customPlot->xAxis->setLabelFont(font);
    m_customPlot->xAxis->setLabelColor(color);
}

void CustomGraph::setAxisLabelFontY( const QFont &font, const QColor &color)
{
    m_customPlot->yAxis->setLabelFont(font);
    m_customPlot->yAxis->setLabelColor(color);
}

void CustomGraph::setAxisLabelFont( const QFont &font, const QColor &color)
{
    m_customPlot->xAxis->setLabelFont(font);
    m_customPlot->xAxis->setLabelColor(color);
    m_customPlot->yAxis->setLabelFont(font);
    m_customPlot->yAxis->setLabelColor(color);
}

void CustomGraph::setAxisTicksLabelFontX( const QFont &font, const QColor &color)
{
    m_customPlot->xAxis->setTickLabelFont(font);
    m_customPlot->xAxis->setTickLabelColor(color);
}

void CustomGraph::setAxisTicksLabelFontY( const QFont &font, const QColor &color)
{
    m_customPlot->yAxis->setTickLabelFont(font);
    m_customPlot->yAxis->setTickLabelColor(color);
}

void CustomGraph::setAxisTicksLabelFont( const QFont &font, const QColor &color)
{
    m_customPlot->xAxis->setTickLabelFont(font);
    m_customPlot->xAxis->setTickLabelColor(color);
    m_customPlot->yAxis->setTickLabelFont(font);
    m_customPlot->yAxis->setTickLabelColor(color);
}

void CustomGraph::setLegendFont( const QFont &font, const QColor &color)
{
    m_customPlot->legend->setFont(font);
    m_customPlot->legend->setTextColor(color);
}

void CustomGraph::setTitle(const QString &str)
{
    auto * title = qobject_cast<QCPTextElement *> (m_customPlot->plotLayout()->element(0,0));
    if(title)
    {
        m_customPlot->plotLayout()->remove(title);
        m_customPlot->plotLayout()->simplify();
        m_customPlot->plotLayout()->insertRow(0);
        m_customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(m_customPlot,str));
    }
    else
    {
        title = new QCPTextElement(m_customPlot,str);
        m_customPlot->plotLayout()->simplify();
        m_customPlot->plotLayout()->insertRow(0);
        m_customPlot->plotLayout()->addElement(0, 0, title);
    }
    m_customPlot->replot();
    connect(title, &QCPTextElement::doubleClicked, this, &CustomGraph::titleDoubleClick);
}

void CustomGraph::setTitle(const QString &str, const QFont &font, const QColor &color )
{
    auto * title = qobject_cast<QCPTextElement *> (m_customPlot->plotLayout()->element(0,0));
    if(title )
    {
        m_customPlot->plotLayout()->remove(title);
        m_customPlot->plotLayout()->simplify();
        if(str!="")
        {
            title->setText(str);
            title->setFont(font);
            title->setTextColor(color);
        }
    }
    else
    {
        m_customPlot->plotLayout()->simplify();
        m_customPlot->plotLayout()->insertRow(0);
        title = new QCPTextElement(m_customPlot,str);
        title->setFont(font);
        title->setTextColor(color);
        m_customPlot->plotLayout()->addElement(0, 0, title);
    }
    m_customPlot->replot();
    connect(title, SIGNAL(doubleClicked(QMouseEvent*)), this, SLOT(titleDoubleClick(QMouseEvent*)));
}

void CustomGraph::titleDoubleClick(QMouseEvent* event)
{
    Q_UNUSED(event)
    if (auto *title = dynamic_cast<QCPTextElement*>(sender()))
    {
        bool ok;
        QString newTitle = QInputDialog::getText(this, "Set Title", "New plot title:", QLineEdit::Normal, title->text(), &ok);
        if (ok)
        {
            title->setText(newTitle);
            m_customPlot->replot();
        }
    }
}

void CustomGraph::selectionChanged()
{
    for (int i=0; i<m_customPlot->graphCount(); ++i)
    {
        setSelectChtLineStyle(i);
    }
}

void CustomGraph::setSelectChtLineStyle(int sceneIndex)
{
    QCPGraph *graph = m_customPlot->graph(sceneIndex);
    QCPPlottableLegendItem *item = m_customPlot->legend->itemWithPlottable(graph);

    if(!graph || !item)
    {
        return;
    }

    if (item->selected() || graph->selected())
    {
        item->setSelected(true);

        QPen pen;
        pen.setWidth(2);
        pen.setColor(Qt::blue);
        graph->selectionDecorator()->setPen(pen);
        graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
    }
}

void CustomGraph::holdOn()
{
    m_isHoldOn = true;
}

void CustomGraph::holdOff()
{
    m_isHoldOn = false;
}

void CustomGraph::legendOn()
{
    if(m_customPlot->legend->itemCount()==0)
        m_customPlot->legend->setVisible(false);
    else
        m_customPlot->legend->setVisible(true);
    m_customPlot->replot();
}

void CustomGraph::legendOff()
{
    m_customPlot->legend->setVisible(false);
    m_customPlot->replot();
}

void CustomGraph::gridOn()
{
    m_customPlot->xAxis->grid()->setVisible(true);
    m_customPlot->yAxis->grid()->setVisible(true);
    m_customPlot->replot();
}

void CustomGraph::gridOff()
{
    m_customPlot->xAxis->grid()->setVisible(false);
    m_customPlot->yAxis->grid()->setVisible(false);
    m_customPlot->replot();
}

void CustomGraph::boxOn()
{
    m_customPlot->axisRect()->setVisible(true);
    m_customPlot->axisRect()->setupFullAxesBox(true);
    m_customPlot->xAxis2->setTicks(false);
    m_customPlot->yAxis2->setTicks(false);
    m_customPlot->replot();
}

void CustomGraph::boxOff()
{
    m_customPlot->xAxis->setVisible(false);
    m_customPlot->yAxis->setVisible(false);
    m_customPlot->xAxis2->setVisible(false);
    m_customPlot->yAxis2->setVisible(false);
    m_customPlot->replot();
}

void CustomGraph::curveSelectOn()
{
    if(!m_curveSelectionGroup)
    {
        m_curveSelectionGroup = new QButtonGroup(this);
        m_curveSelectionGroup->setExclusive(false);
        connect(m_curveSelectionGroup, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
                this, &CustomGraph::curveSelectionChanged);
    }

    m_curveSelectionLayout->setEnabled(true);
}

void CustomGraph::curveSelectOff()
{
    m_curveSelectionLayout->setEnabled(false);
}

void CustomGraph::curveSelectionChanged(int index, bool status)
{
    if(index<0 || index>=m_customPlot->graphCount())
    {
        return;
    }

    if(!m_customPlot->graph(index))
    {
        return;
    }
    m_customPlot->graph(index)->setVisible(status);
    m_customPlot->replot();
}

void CustomGraph::setCurveSelectionBtnGroup(const QStringList& names)
{
    if(names.isEmpty())
    {
        return;
    }

    int nameSize = names.size();
    int btnSize  = m_curveSelectionGroup->buttons().size();
    while(btnSize != nameSize)
    {
        if(btnSize > nameSize)
        {
            auto btn = m_curveSelectionGroup->button(btnSize-1);
            m_curveSelectionGroup->removeButton(btn);
            delete btn;
        }
        else if(btnSize < nameSize)
        {
            auto chk = new QCheckBox(this);
            m_curveSelectionGroup->addButton(chk, btnSize);
            m_curveSelectionLayout->addWidget(chk);
            chk->setCheckState(Qt::Checked);
        }
        btnSize  = m_curveSelectionGroup->buttons().size();
    }

    for(int i=0; i<nameSize; i++)
    {
        m_curveSelectionGroup->button(i)->setText(names.at(i));
    }
}

void CustomGraph::mousePress(QMouseEvent *event)
{
//    if(event->button() == Qt::LeftButton)
//    {
//        double x = m_customPlot->xAxis->pixelToCoord(event->pos().x());
//        double y = m_customPlot->yAxis->pixelToCoord(event->pos().y());
//
//        m_scatterGraph->addData(x, y);
//        m_customPlot->replot();
//    }

    // BmTODO 切换曲线时有问题
    if(event->button() == Qt::LeftButton)
    {
        if(m_currMode == AddMarker && m_addMarkerFinish)
        {
            QList<QCPGraph*> graphs = m_customPlot->selectedGraphs();
            if(graphs.isEmpty())
            {
                return;
            }
            m_addMarkerFinish = false;
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        QPoint pos = mapToGlobal(event->pos());
        m_contextMenu->exec(pos);
    }
}

void CustomGraph::mouseRelease(QMouseEvent *event)
{

}

void CustomGraph::mouseDoubleClick(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        if(m_customPlot->axisRect()->rect().contains(event->pos()) && (m_customPlot->axisRect()->insetLayout()->selectTest(event->pos(),false) <0) )
            m_customPlot->rescaleAxes();
        m_customPlot->replot();
    }
}

void CustomGraph::mouseMove(QMouseEvent *event)
{
    QList<QCPGraph*> graphs = m_customPlot->selectedGraphs();
    if(graphs.isEmpty())
    {
        m_tracer->setVisible(false);
        m_tracerLabel->setVisible(false);
        return;
    }

    if(m_currMode == AddMarker)
    {
        m_tracer->setVisible(false);
        m_tracerLabel->setVisible(false);

        if(!m_addMarkerFinish){
            // 设置游标
            auto *marker = new QCPItemTracer(m_customPlot);
            marker->setInterpolating(true);
            marker->setStyle(QCPItemTracer::tsCircle);
            marker->setPen(QPen(Qt::red));
            marker->setBrush(Qt::red);
            marker->setSize(8);
            marker->setVisible(false);

            // 游标说明s
            auto *markerLabel = new QCPItemText(m_customPlot);
            markerLabel->setPen(QPen(Qt::red));
            markerLabel->setFont(QFont(font().family(), 14));
            markerLabel->setPositionAlignment(Qt::AlignRight | Qt::AlignBottom);
            markerLabel->position->setParentAnchor(marker->position);
            markerLabel->setVisible(false);

            m_markers.push_back(QPair<QCPItemTracer*, QCPItemText*>(marker, markerLabel));

            m_addMarkerFinish = true;
        }

        auto marker = m_markers.last().first;
        auto markerLabel = m_markers.last().second;

        marker->setVisible(true);
        markerLabel->setVisible(true);

        marker->setGraphKey(m_customPlot->xAxis->pixelToCoord(event->pos().x()));
        markerLabel->setText(QString("M"));

        // 将锚点设置到选中的第一个graph上
        marker->setGraph(graphs.first());

        m_customPlot->replot();

        return;
    }

    m_tracer->setVisible(true);
    m_tracerLabel->setVisible(true);
    m_tracer->setGraphKey(m_customPlot->xAxis->pixelToCoord(event->pos().x()));

    double xValue = m_tracer->position->key();
    double yValue = m_tracer->position->value();
    m_tracerLabel->setText(QString("x = %1, y = %2").arg(xValue).arg(yValue));

    // 将锚点设置到选中的第一个graph上
    m_tracer->setPen(graphs.first()->pen());
    m_tracerLabel->setPen(graphs.first()->pen());
    m_tracer->setGraph(graphs.first());

    m_customPlot->replot();
}

void CustomGraph::onPlotClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event)
{
    if(event->button() != Qt::LeftButton)
    {
        return;
    }

    const QCPGraphData *ghd = dynamic_cast<QCPGraph*>(plottable)->data()->at(dataIndex);
    for(int i=0; i<m_customPlot->graphCount(); i++)
    {
        if(m_customPlot->graph(i)->selected())
        {
            CoordInputDialog dialog;
            if(QDialog::Accepted == dialog.exec())
            {
                m_customPlot->graph(i)->data()->remove(ghd->mainKey());
                std::cout << "x:" << dialog.getCoordinateX() << "y:" << dialog.getCoordinateY() << std::endl;
                m_customPlot->graph(i)->addData(dialog.getCoordinateX(), dialog.getCoordinateY());
            }
        }
    }
    m_customPlot->replot(QCustomPlot::rpQueuedReplot);
    std::cout << "ghd:" << ghd->mainKey() << " " << ghd->mainValue() << std::endl;
}

void CustomGraph::clear()
{
    m_customPlot->clearGraphs();
    m_customPlot->replot();
}

void CustomGraph::setTextItem(const QString &text)
{
    if(!m_textItem)
    {
        m_textItem = new QCPItemText(m_customPlot);
        m_textItem->setPositionAlignment(Qt::AlignBottom|Qt::AlignLeft);
        m_textItem->position->setType(QCPItemPosition::ptAxisRectRatio);
        m_textItem->position->setCoords(0.03, 0.97);
        m_textItem->setFont(QFont(font().family(), 14));
    }

    m_textItem->setText(text);
    m_textItem->setVisible(true);
}

void CustomGraph::addMarker()
{
    if(m_addMarkerAction->text() == tr("Add Marker"))
    {
        m_addMarkerAction->setText("End Marker Mode");
        m_currMode = AddMarker;
    }
    else if(m_addMarkerAction->text() == tr("End Marker Mode"))
    {
        m_addMarkerAction->setText("Add Marker");
        m_currMode = NoMode;

        m_addMarkerFinish = false;
    }
}

CoordInputDialog::CoordInputDialog(QWidget *parent)
    :QDialog(parent)
{
    setFixedSize(200, 100);
    initUi();

    connect(m_btn_ok, &QPushButton::clicked, this, &CoordInputDialog::onBtnOkClicked);
    connect(m_spinBox_x, SIGNAL(valueChanged(double)), this, SLOT(setCoordinateX(double)));
    connect(m_spinBox_y, SIGNAL(valueChanged(double)), this, SLOT(setCoordinateY(double)));
}

CoordInputDialog::~CoordInputDialog()
= default;

void CoordInputDialog::initUi()
{
    auto *xLabel = new QLabel("X:", this);
    m_spinBox_x = new QDoubleSpinBox(this);
    m_spinBox_x->setSingleStep(0.01);
    auto *yLabel = new QLabel("Y:", this);
    m_spinBox_y = new QDoubleSpinBox(this);
    m_spinBox_y->setSingleStep(0.01);
    m_btn_ok = new QPushButton("OK", this);

    auto *layout = new QGridLayout(this);
    layout->addWidget(xLabel, 0, 0, 1, 1);
    layout->addWidget(m_spinBox_x, 0, 1, 1, 2);
    layout->addWidget(yLabel, 1, 0, 1, 1);
    layout->addWidget(m_spinBox_y, 1, 1, 1, 2);
    layout->addWidget(m_btn_ok, 2, 2, 1, 1);
}

void CoordInputDialog::onBtnOkClicked()
{
    QDialog::accept();
    close();
}
