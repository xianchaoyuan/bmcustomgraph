#include <QApplication>
#include <QVector>
#include "CustomGraph.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

        QPen pen, pen1, pen2, pen3;
    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    pen.setColor(Qt::black);

    pen1.setStyle(Qt::DashLine);
    pen1.setWidth(2);
    pen1.setColor(Qt::red);

    pen2.setStyle(Qt::DotLine);
    pen2.setWidth(2);
    pen2.setColor(Qt::blue);

    pen3.setStyle(Qt::DashDotLine);
    pen3.setWidth(2);
    pen3.setColor(Qt::darkMagenta);

    // 要绘制的数据
    QVector<double> xs, ys;
    for (double x = 0.0; x < 2.0*M_PI; x += M_PI/300) {
        xs.append(x);
        ys.append(sin(x));
    }
    QVector<double> xs1, ys1;
    for (double x = 0.0; x < 2.0*M_PI; x += M_PI/200) {
        xs1.append(x);
        ys1.append(sin(x)+1);
    }
    QVector<double> xs2, ys2;
    for (double x = 0.0; x < 2.0*M_PI; x += M_PI/50) {
        xs2.append(x);
        ys2.append(sin(x)-1);
    }

    CustomGraph gw;
    gw.setTitle("CustomGraph");
    gw.setFixedSize(800, 600);
    gw.boxOn();
    gw.holdOn();
    gw.legendOn();

    gw.setAxisRangeX(-0.2, 2.0 * M_PI + 0.2);
    gw.setAxisRangeY(-2.2, 2.2);

    gw.plot(xs, ys, pen, "");
    gw.plot(xs1, ys1, pen1, "Graph 2");
    //    gw.plotScatters(xs2, ys2, QCPScatterStyle(QCPScatterStyle::ssCircle, Qt::red, 8), "Scatters1");

    gw.addGoal(1, 1, 5, 1, Goal::LessAndEqual);
    gw.addGoal(1, -1, 5, 0, Goal::Minimun);

    gw.show();

    return QApplication::exec();
}
