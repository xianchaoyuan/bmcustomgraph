#ifndef CUSTOMGRAPH_H
#define CUSTOMGRAPH_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include "QCustomPlot.h"

/**
 * @brief goal线绘制
 */
struct Goal
{
    enum Condition
    {
        NoCnd = 0,
        LessAndEqual,
        Equal,
        GrantAndEqual,
        Minimun,
        Maximum
    };

    QPointer<QCPGraph> line{ nullptr };
    QPointer<QCPGraph> lineTop{ nullptr };
    QPointer<QCPGraph> lineBottom{ nullptr };
    Condition cnd{NoCnd};
    QPointF start;
    QPointF stop;
};

/**
 * @brief 坐标设置
 */
class CoordInputDialog : public QDialog
{
Q_OBJECT
    Q_PROPERTY(double CoordinateX READ getCoordinateX WRITE setCoordinateX NOTIFY coordinateXChanged)
    Q_PROPERTY(double CoordinateY READ getCoordinateY WRITE setCoordinateY NOTIFY coordinateYChanged)
public:
    explicit CoordInputDialog(QWidget *parent = nullptr);
    ~CoordInputDialog() override;

    double getCoordinateX() const{ return m_coordinate_x; }
    double getCoordinateY() const{ return m_coordinate_y; }

signals:
    void coordinateXChanged();
    void coordinateYChanged();

private slots:
    void setCoordinateX(double value){ m_coordinate_x=value; }
    void setCoordinateY(double value){ m_coordinate_y=value; }

    void onBtnOkClicked();

private:
    void initUi();

private:
    QDoubleSpinBox *m_spinBox_x{nullptr};
    QDoubleSpinBox *m_spinBox_y{nullptr};

    QPushButton *m_btn_ok{nullptr};

    double m_coordinate_x{0};
    double m_coordinate_y{0};
};


/**
*    @brief 自定义曲线图
*/
class CustomGraph : public QWidget
{
Q_OBJECT
public:
    enum Mode
    {
        AddMarker,
        NoMode,
    };

    explicit CustomGraph(QWidget *parent = nullptr);
    ~CustomGraph() override;

    //! 初始化Ui
    void initUi();

    //! goal线绘制
    void addGoal(double xStart, double yStart, double xEnd, double yEnd, Goal::Condition cnd = Goal::Equal, const QColor &color = Qt::red);

    void plot(const QVector<QVector<double>>& data, const QStringList& legendNames = {});
    void plot(const QVector<double>& x, const QVector<double>& y);
    void plot(const QVector<double>& x, const QVector<double>& y, const QPen& pen, const QString & legendName = "");
    void plot(const QVector<double>& x, const QVector<double>& y, const QPen& pen, const QCPScatterStyle & style, const QString & legendName = "");
    void plotScatters(const QVector<double>& x, const QVector<double>& y, const QCPScatterStyle & style, const QString & legendName = "");

    void setAxisRangeX(double lower, double upper);
    void setAxisRangeY(double lower, double upper);
    void setAxisLabelX(const QString &str);
    void setAxisLabelY(const QString &str);
    void setAxisLabelX(const QString &str, const QFont &font, const QColor &color = Qt::black);
    void setAxisLabelY(const QString &str, const QFont &font, const QColor &color = Qt::black);
    void setAxisLabelFontX( const QFont &font, const QColor &color = Qt::black);
    void setAxisLabelFontY( const QFont &font, const QColor &color = Qt::black);
    void setAxisLabelFont( const QFont &font, const QColor &color = Qt::black);
    void setAxisTicksLabelFontX( const QFont &font, const QColor &color = Qt::black);
    void setAxisTicksLabelFontY( const QFont &font, const QColor &color = Qt::black);
    void setAxisTicksLabelFont( const QFont &font, const QColor &color = Qt::black);

    void setLegendFont( const QFont &font, const QColor &color = Qt::black);

    void setTitle(const QString &str);
    void setTitle(const QString &str, const QFont &font, const QColor &color = Qt::black);
    void titleDoubleClick(QMouseEvent* event);

    void selectionChanged();
    void setSelectChtLineStyle(int sceneIndex);

    //! 曲线覆盖
    void holdOn();
    void holdOff();

    //! 图例开关
    void legendOn();
    void legendOff();

    //! 表格开关
    void gridOn();
    void gridOff();

    //! box开关
    void boxOn();
    void boxOff();

    //! 曲线选择开关
    void curveSelectOn();
    void curveSelectOff();
    void curveSelectionChanged(int index, bool status);
    void setCurveSelectionBtnGroup(const QStringList& names);

    //! 清空
    void clear();

    //! 设置提示文本
    void setTextItem(const QString& text);

public:
    //! 添加marker
    void addMarker();

protected:
    void mousePress(QMouseEvent *event);
    void mouseRelease(QMouseEvent *event);
    void mouseDoubleClick(QMouseEvent *event);
    void mouseMove(QMouseEvent *event);

    //! 选点处理
    void onPlotClick(QCPAbstractPlottable *plottable, int dataIndex, QMouseEvent *event);

private:
    QVector<QPair<QCPItemTracer*, QCPItemText*>> m_markers;

    Mode m_currMode{NoMode};
    QMenu   *m_contextMenu{nullptr};
    QAction *m_addMarkerAction{nullptr};

    QCustomPlot   *m_customPlot{nullptr};
    QCPItemTracer *m_tracer{nullptr};
    QCPItemText   *m_tracerLabel{nullptr};
    QCPItemText   *m_textItem{nullptr};        //textItem

    QCPGraph *m_scatterGraph{nullptr};

    QButtonGroup *m_curveSelectionGroup{nullptr};
    QVBoxLayout  *m_curveSelectionLayout{nullptr};

    bool m_isHoldOn{false};
    bool m_addMarkerFinish{false};

};
#endif // CUSTOMGRAPH_H
