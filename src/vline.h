#ifndef VLINE_H
#define VLINE_H

#include "qcustomplot.h"


enum class ColorTheme
{
    Light,
    Dark,
    Red,
    TOS
};

class VLine : public QCPItemLine
{

public:
    VLine(QCustomPlot *parentPlot, ColorTheme color);
    ~VLine();

    void UpdateLabel(double x, double y, QString text);
    void SetVisibleCustom(bool value);

    QCPItemText* m_lineLabel;
};

#endif // VLINE_H
