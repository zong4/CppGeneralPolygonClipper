#ifdef RELEASE

#include "qnamespace.h"
#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>

#include "gpc.hpp"

class PolygonWidget : public QWidget
{
public:
    PolygonWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMouseTracking(true);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && draw_state != -1)
        {
            points.vertexs.push_back(
                {event->position().x(), event->position().y()});
            update();
            return;
        }

        if (event->button() == Qt::RightButton)
        {
            if (draw_state == 0)
            {
                subj_polygon.add_contour(points);
                points.vertexs.clear();

                draw_state = -1;
            }
            else if (draw_state == 1)
            {
                clip_polygon.add_contour(points);
                points.vertexs.clear();

                draw_state = -1;
            }

            update();
            return;
        }
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_1)
        {
            draw_state = 0;
        }
        else if (event->key() == Qt::Key_2)
        {
            draw_state = 1;
        }
        else if (event->key() == Qt::Key_R)
        {
            subj_polygon.contours.clear();
            clip_polygon.contours.clear();
            result_polygon.contours.clear();
        }
        else if (event->key() == Qt::Key_D)
        {
            performBooleanOperation();
        }

        update();
    }

    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);

        if (result_polygon.num_contours() > 0)
        {
            painter.setPen(QPen(Qt::red, 3));
            for (auto &&contour : result_polygon.contours)
            {
                for (int i = 0; i < contour.num_vertices(); i++)
                {
                    const auto &v1 = contour.vertexs[i];
                    const auto &v2 =
                        contour.vertexs[(i + 1) % contour.num_vertices()];

                    painter.drawLine(v1.x, v1.y, v2.x, v2.y);
                }
            }

            return;
        }

        if (points.num_vertices() > 0)
        {
            painter.setPen(Qt::black);
            for (int i = 0; i < points.num_vertices() - 1; i++)
            {
                const auto &v1 = points.vertexs[i];
                const auto &v2 =
                    points.vertexs[(i + 1) % points.num_vertices()];

                painter.drawLine(v1.x, v1.y, v2.x, v2.y);
            }
        }

        if (subj_polygon.num_contours() > 0)
        {
            painter.setPen(Qt::green);
            for (auto &&contour : subj_polygon.contours)
            {
                for (int i = 0; i < contour.num_vertices(); i++)
                {
                    const auto &v1 = contour.vertexs[i];
                    const auto &v2 =
                        contour.vertexs[(i + 1) % contour.num_vertices()];

                    painter.drawLine(v1.x, v1.y, v2.x, v2.y);
                }
            }
        }

        if (clip_polygon.num_contours() > 0)
        {
            painter.setPen(Qt::blue);
            for (auto &&contour : clip_polygon.contours)
            {
                for (int i = 0; i < contour.num_vertices(); i++)
                {
                    const auto &v1 = contour.vertexs[i];
                    const auto &v2 =
                        contour.vertexs[(i + 1) % contour.num_vertices()];

                    painter.drawLine(v1.x, v1.y, v2.x, v2.y);
                }
            }
        }
    }

private:
    int draw_state = -1; // 0: draw subject, 1: draw clip
    gpc::gpc_vertex_list points;
    gpc::gpc_polygon subj_polygon;
    gpc::gpc_polygon clip_polygon;
    gpc::gpc_polygon result_polygon;

    void performBooleanOperation()
    {
        gpc::gpc_polygon_clip(gpc::gpc_op::GPC_UNION, subj_polygon,
                              clip_polygon, result_polygon);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    PolygonWidget widget;
    widget.show();
    return app.exec();
}

#endif