#include "vline.h"

VLine::VLine(QCustomPlot *parentPlot, ColorTheme color)
    : QCPItemLine(parentPlot)
{
    m_lineLabel = new QCPItemText(parentPlot);
    m_lineLabel->setText("");
    m_lineLabel->setVisible(false);
    m_lineLabel->setSelectable(false);

    QPen pen,penSel;
    pen.setBrush(Qt::white);
    pen.setWidth(2);
    penSel.setBrush(Qt::white);
    penSel.setWidth(5);
    if (color == ColorTheme::Light)
    {
        pen.setColor(Qt::lightGray);
        penSel.setColor(QColor(255, 50, 10));
        m_lineLabel->setColor(Qt::darkGray);
    }
    else if(color == ColorTheme::Dark)
    {
        pen.setColor(QColor(100, 100, 100));
        penSel.setColor(QColor(255, 50, 10));
        m_lineLabel->setColor(Qt::lightGray);
    }
    else if(color == ColorTheme::Red)
    {
        pen.setColor(QColor(255, 50, 10));
        penSel.setColor(QColor(255, 50, 10));
        m_lineLabel->setColor(QColor(255, 50, 10));
    }

    this->setVisible(false);
    this->setPen(pen);
    this->setSelectable(true);
    this->mSelectedPen = penSel;

}
VLine::~VLine(){
    parentPlot()->removeItem(m_lineLabel);
}

void VLine::UpdateLabel(double x, double y, QString text)
{
    m_lineLabel->setText(text);
    m_lineLabel->position->setAxes(this->positions().first()->keyAxis(),this->positions().first()->valueAxis());
    m_lineLabel->position->setType(QCPItemPosition::ptPlotCoords);
    m_lineLabel->position->setCoords(x, y);
}

void VLine::SetVisibleCustom(bool value)
{
    m_lineLabel->setVisible(value);
    this->setVisible(value);
}
