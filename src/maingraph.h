#ifndef MAINGRAPH_H
#define MAINGRAPH_H

#include <QMainWindow>
#include <QtWidgets/QWidget>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QChar>
#include <QPair>

#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <list>

#include "vline.h"
#include "qcustomplot.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainGraph; }
QT_END_NAMESPACE

//QT_CHARTS_BEGIN_NAMESPACE
//class QChartView;
//class QChart;
//QT_CHARTS_END_NAMESPACE
//QT_CHARTS_USE_NAMESPACE

typedef QList<QPointF> DataList;

struct Label{
    QString label;
    char key;
    bool operator<(Label const & that) const {
        if(key==that.key)
            return label.length()<that.label.length();
        else
           return key<that.key;
   }
   bool operator == (Label const & that) const {
        return (key == that.key || label == that.label);
   }
};

struct LabelData{
    Label lbl;
    double  t;
    VLine *line;
    bool operator<(LabelData const & that) const {
           return t<that.t;
   }
    bool operator == (LabelData const & that) const {
         return (t == that.t && lbl.key == that.lbl.key);
    }
};


class MainGraph : public QMainWindow
{
    Q_OBJECT

public:
    MainGraph(QWidget *parent = nullptr);
    MainGraph(QApplication *app, QWidget *parent = nullptr);
    ~MainGraph();
    void drawLineChart();
    void loadFile(QString file);
    void openAndLoadAnnotation(QString fileName);
    void saveAndUpdate();
    void save();
    void addToSaveList(Label inLbl);
    void removeFromSaveList();
    void addLabelDataToPlot(LabelData lblToAdd);
    double findAvgLastNPoints(double key, unsigned long pointNumber = 100);
    void reset(bool onlyAnnotation = false);

private slots:

    void on_sourcePath_returnPressed();

    void on_findSource_clicked();

    void on_saveFilePath_returnPressed();

    void on_Save_clicked();

    void on_findSaveLocation_clicked();

    void on_addLbl_clicked();

    //MINE:

    void on_QCPMousePressed_pressed(QMouseEvent *);

    void on_selection_changed_global();

    void on_mouse_move(QMouseEvent* e);

    void on_QCPMouseReleased_release(QMouseEvent* );


private:
    Ui::MainGraph *ui;
    QApplication *baseApp;



    //Test CustomPlot
    QVector<QPair<double,double>> data;
    QCustomPlot *plt;

    //Files and lbl relative variables
    bool fileLoaded;
    bool newSaveFile;
    bool isMultiSelection;
    bool dragging;
    QString fileName;
    QString fileToSave;
    QFile annotationFile;
    //QTextStream annotationStream;
    std::vector<Label> keyLbl;
    uint8_t nextLblPos;
    std::list<double> selected;
    std::list<LabelData> lbldData; //This will hold ALL the current labels always
    LabelData *hovered;

    QCPItemText *nextLabel;

protected:
    void keyPressEvent(QKeyEvent *e);

};
#endif // MAINGRAPH_H
