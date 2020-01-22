#include "maingraph.h"
#include "ui_maingraph.h"



using namespace QtCharts;

MainGraph::MainGraph(QApplication *app, QWidget *parent)
    : QMainWindow(parent),
      fileLoaded(0),
      newSaveFile(false),
      keyLbl(),
      data(),
      baseApp(app),
      ui(new Ui::MainGraph)
{
    ui->setupUi(this);
    plt = new QCustomPlot(ui->centralwidget);
    nextLabel = new QCPItemText(plt);
    nextLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignRight);
    nextLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    nextLabel->position->setCoords(0.998, 0); // place position at center/top of axis rect
    nextLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    nextLabel->setText("");
    //nextLabel->setPen(QPen(Qt::black)); // show black border around text


    ui->appLayout->insertWidget(0,plt);
    ui->appLayout->setStretch(0,5);
    ui->appLayout->setStretch(1,1);

    //catch ctrl --> window zoom or pan
    QObject::connect(plt,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(on_QCPMousePressed_pressed(QMouseEvent*)));
    QObject::connect(plt,SIGNAL(selectionChangedByUser()),this,SLOT(on_selection_changed_global()));
    QObject::connect(plt, SIGNAL(mouseMove(QMouseEvent*)), this,SLOT(on_mouse_move(QMouseEvent*)));
    QObject::connect(plt,SIGNAL(mouseRelease(QMouseEvent*)),this,SLOT(on_QCPMouseReleased_release(QMouseEvent*)));


}


MainGraph::~MainGraph()
{
    delete ui;
    if(annotationFile.isOpen()){
        annotationFile.close();
    }

}

void MainGraph::drawLineChart()
{
    QVector<double> x,y;
    uint minRange = 10000, maxRange = 0, tRange = 0;
    const int numPointToDisplay = 100;
    int i = 0;

    ui->saveFilePath->setEnabled(true);
    ui->Save->setEnabled(true);
    ui->findSaveLocation->setEnabled(true);

    for (int i = 0; i < plt->graphCount();i++)
        plt->removeGraph(i);

    for(QVector<QPair<uint,uint>>::iterator iter = data.begin(); iter != data.end(); iter ++,i++){
        x.append(iter->first);
        y.append(iter->second);
        if(i<numPointToDisplay){
            if(iter->second<minRange)
                minRange = iter->second;
            if(iter->second > maxRange)
                maxRange = iter->second;
            tRange = iter->first;
        }
    }


    plt->addGraph(plt->xAxis, plt->yAxis);
    plt->graph(0)->setData(x, y);

    plt->setInteraction(QCP::iRangeDrag, true);
    plt->setInteraction(QCP::iRangeZoom, true);
    plt->setInteraction(QCP::iSelectPlottables, true);
    plt->setInteraction(QCP::iMultiSelect);
    plt->setInteraction(QCP::iSelectItems);
    //plt->graph(0)->setSelectable(QCP::stSingleData);
    plt->graph(0)->setSelectable(QCP::stMultipleDataRanges);

    //plt->addGraph(plt->yAxis, plt->xAxis);
    plt->graph(0)->setPen(QPen(QColor(0, 25, 255)));
    plt->graph(0)->setLineStyle(QCPGraph::lsLine);
    plt->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));

    //Decorator for the SELECTED point
    QCPScatterStyle selectedStyle;
    selectedStyle.setShape(QCPScatterStyle::ssTriangle);
    selectedStyle.setPen(QPen(QColor(255,10,4)));
    selectedStyle.setBrush(Qt::white);
    selectedStyle.setSize(5);
    plt->graph(0)->selectionDecorator()->setScatterStyle(selectedStyle,QCPScatterStyle::spAll);

    // give the axes some labels:
    plt->xAxis->setLabel("Time");
    plt->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    plt->xAxis->setRange(-15, tRange);
    plt->yAxis->setRange(minRange-15, maxRange+15);

    plt->replot();

    //SELECTION
    QObject::connect(plt,SIGNAL(itemClick(QCPAbstractItem *, QMouseEvent *)),this,SLOT(on_item_click(QCPAbstractItem *, QMouseEvent *)));
}




void MainGraph::on_findSource_clicked()
{
    if (!fileName.isEmpty())
        fileName = QFileDialog::getOpenFileName(this, "Open File",fileName);
    else
        fileName = QFileDialog::getOpenFileName(this, "Open File","/home/zanoli/Desktop/ECG_date2020_finalSubmssion/Event_based_gQRS/data/dataOut/lvlCrossingBits6Hist0/");
    if (fileName.isEmpty())
        return;
    ui->sourcePath->setText(fileName);
    ui->sourcePath->setCursorPosition(0);
    loadFile(fileName);
}

void MainGraph::on_sourcePath_returnPressed()
{
    if (ui->sourcePath->text().isEmpty())
        return;
    QFileInfo check_file(ui->sourcePath->text());
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile()) {
        fileName = ui->sourcePath->text();
    } else if(check_file.exists() && check_file.isDir()) {
        fileName = QFileDialog::getOpenFileName(this, "Open File",ui->sourcePath->text());
        if (!fileName.isEmpty()){
            ui->sourcePath->setText(fileName);
            ui->sourcePath->setCursorPosition(0);
        }
    }
    if (fileName.isEmpty())
        return;
    loadFile(fileName);
}

void MainGraph::loadFile(QString fileName){
    //save and clear the previous data file and load the new one
    save();
    if (ui->key->isEnabled()){
        ui->key->setEnabled(false);
        ui->lbl->setEnabled(false);
        ui->addLbl->setEnabled(false);
        ui->lblKeyList->setEnabled(false);
    }
    ui->lblKeyList->clear();
    ui->lblKeyList->update();
    if(annotationFile.isOpen()){
        annotationFile.close();
    }

    data.clear();
    fileLoaded = 0;
    QString line;
    QStringList wordList;
    if(fileName.indexOf(".csv") != -1){
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            throw "Impossible to read the selected file";
        QTextStream in(&file);

        while (!in.atEnd()) {
            line = in.readLine();
            wordList = line.split(',');
            QPointF value(wordList[0].toInt(),wordList[1].toInt());

            QPair<uint,uint> vectorThisPoint(wordList[0].toInt(),wordList[1].toInt());
            data << vectorThisPoint;
        }
        file.close();
    }
    fileLoaded = 1;
    drawLineChart();
}


void MainGraph::on_saveFilePath_returnPressed()
{
    QString dir;
    if (ui->saveFilePath->text().isEmpty())
        return;
    QFileInfo check_file(ui->saveFilePath->text());
    // check if file exists and if yes: Is it really a file and no directory?
    if (check_file.exists() && check_file.isFile() && check_file.isWritable()) {
        fileToSave = QFileDialog::getSaveFileName(this,"Save File",ui->saveFilePath->text());
    } else if(check_file.exists() && check_file.isDir()) {
        fileToSave = QFileDialog::getSaveFileName(this, "Save File",ui->saveFilePath->text());
        if (!fileToSave.isEmpty()){
            ui->saveFilePath->setText(fileToSave);
            ui->saveFilePath->setCursorPosition(0);
        }
    } else if (!check_file.exists() && QFileInfo(check_file.dir().canonicalPath()).isWritable()){
        //inserted the complete name of a new file in an existing directory
        fileToSave = QFileDialog::getSaveFileName(this,"Save File",ui->saveFilePath->text());
        ui->saveFilePath->setText(fileToSave);
        ui->saveFilePath->setCursorPosition(0);
    }
    if (fileToSave.isEmpty())
        return;
    openAndLoadAnnotation(fileToSave);
}

void MainGraph::on_findSaveLocation_clicked()
{
    //may be a problem, what happen if we select a file in a not-accessible path?
    if (!fileToSave.isEmpty())
        fileToSave = QFileDialog::getSaveFileName(this, "Save File",fileToSave);
    else
        fileToSave = QFileDialog::getSaveFileName(this, "Save File","/home/zanoli/Desktop/tempo");
    if (fileToSave.isEmpty())
        return;
    ui->saveFilePath->setText(fileToSave);
    ui->saveFilePath->setCursorPosition(0);
    openAndLoadAnnotation(fileToSave);
}

void MainGraph::openAndLoadAnnotation(QString fileName){
    //the check on fileName accessibility need to be done before
    //First wa save any possible previously opened labeld file
    save();
    for(auto it = lbldData.begin();it != lbldData.end(); it++)
        plt->removeItem(it->line);
    plt->replot();
    keyLbl.clear();
    lbldData.clear();
    QString line;
    QStringList wordList;
    if(annotationFile.isOpen()){
        annotationFile.close();
        annotationFile.setFileName(fileName);
    }
    else{
        annotationFile.setFileName(fileName);
    }
    //drawLineChart();

    if (annotationFile.exists() && annotationFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        // file already exist, we read it, save the data and reset-it in order to be written again from start
        QTextStream in(&annotationFile);
        while (!in.atEnd()) {
            line = in.readLine().remove(" ");
            if(line.isEmpty())// tollerate blank line
                continue;
            wordList = line.split(',');
            if (wordList.length() != 3)
                throw "The selected file is wrogly formatted";

            Label lbl;
            lbl.label = wordList[1];
            lbl.key = wordList[2].at(0).toLatin1();
            bool present = false;
            for(unsigned long i = 0; i<keyLbl.size();i++)
            {
                if(keyLbl[i]==lbl)
                    present = true;
            }
            if (!present)
                keyLbl.push_back(lbl);
            LabelData dataL = {lbl,wordList[0].toUInt(),new VLine(plt,ColorTheme::Red)};
            lbldData.push_back(dataL);
            newSaveFile = true;
            addLabelDataToPlot(dataL);
        }
        if(keyLbl.size()>0)
        {
            nextLblPos = 0;
            QString nextLblStr = "Next label: ";
            nextLabel->setText(nextLblStr.append(keyLbl[nextLblPos].label));
            plt->replot();

        }
        annotationFile.close();
        annotationFile.open(QIODevice::WriteOnly | QIODevice::Text);
    }
    else
        annotationFile.open(QIODevice::WriteOnly | QIODevice::Text);
    saveAndUpdate();
}

void MainGraph::saveAndUpdate(){
    save();
    if (!ui->key->isEnabled()){
        ui->key->setEnabled(true);
        ui->lbl->setEnabled(true);
        ui->addLbl->setEnabled(true);
        ui->lblKeyList->setEnabled(true);
    }
    ui->lblKeyList->clear();
    std::vector<Label>::iterator itr;
    for(itr=keyLbl.begin(); itr!=keyLbl.end(); itr++){
        QString lab = itr->label;
        lab.append("\t------------------>\t");
        lab.append(itr->key);
        ui->lblKeyList->addItem(lab);
    }
    ui->lblKeyList->update();
}

void MainGraph::on_Save_clicked()
{
    save();
}

void MainGraph::save(){
    if(newSaveFile){
        if(annotationFile.isOpen()){
            annotationFile.close();
        }
        if (annotationFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream in(&annotationFile);
            lbldData.sort();
            for(auto iterSave = lbldData.begin(); iterSave!= lbldData.end(); iterSave++){
                in <<  QString::number(iterSave->t) << ',' << iterSave->lbl.label << ',' << iterSave->lbl.key << '\n';
            }
            newSaveFile = false;
            ui->ev->setText("Saved");
        }
    }
}


void MainGraph::on_addLbl_clicked()
{
    if(!ui->lbl->text().isEmpty() && !ui->key->text().isEmpty()){
        Label newLbl;
        newLbl.key = ui->key->text().at(0).toLatin1();
        newLbl.label = ui->lbl->text();
        for(auto iter = keyLbl.begin();iter != keyLbl.end();iter++){
            if(iter->key==newLbl.key || iter->label==newLbl.label)
                return;
        }
        keyLbl.push_back(newLbl);
        QString lab = newLbl.label;
        lab.append("\t------------------>\t");
        lab.append(newLbl.key);
        ui->lblKeyList->addItem(lab);
        ui->lblKeyList->update();
        if(keyLbl.size() == 1)
        {
            nextLblPos = 0;
            QString nextLblStr = "Next label: ";
            nextLabel->setText(nextLblStr.append(keyLbl[nextLblPos].label));
            plt->replot();
        }
    }
}

void MainGraph::on_QCPMousePressed_pressed(QMouseEvent *event){
    Qt::KeyboardModifiers mod = baseApp->keyboardModifiers();
    bool ctrlPressed = mod.testFlag(Qt::ControlModifier);
    if(!ctrlPressed && selected.size() == 0){
        plt->setSelectionRectMode(QCP::srmNone);
        isMultiSelection = false;
    }
    else if(ctrlPressed)
    {
        plt->setSelectionRectMode(QCP::srmZoom);
        isMultiSelection = true;
    }
    else{
        isMultiSelection = true;
        plt->setSelectionRectMode(QCP::srmNone);
    }

    //check if we are touching a label:
    if(hovered && !isMultiSelection){
        dragging = true;
        plt->setInteraction(QCP::iRangeDrag, false);
    }
    else
        dragging = false;
    plt->replot();
}

void MainGraph::keyPressEvent(QKeyEvent *e){
    if(isMultiSelection && !selected.empty()){
        for(unsigned long i = 0;i<keyLbl.size();i++)
        {
            if (e->text()==keyLbl[i].key){
                nextLblPos = i + 1;
                if(nextLblPos >= keyLbl.size())
                    nextLblPos = 0;
                QString nextLblStr = "Next label: ";
                nextLabel->setText(nextLblStr.append(keyLbl[nextLblPos].label));
                addToSaveList(keyLbl[i]);
                break;
            }
        }
        if(e->key() == Qt::Key_Delete && selected.size() >0){
            removeFromSaveList(); //problem here
        }

    }
    else if(e->key() == Qt::Key_Delete && selected.size() >0){
        removeFromSaveList(); //problem here
    }
    else{
        for(unsigned long i = 0;i<keyLbl.size();i++){
            if (e->text()==keyLbl[i].key){
                nextLblPos = i;
                QString nextLblStr = "Next label: ";
                nextLabel->setText(nextLblStr.append(keyLbl[nextLblPos].label));
                plt->replot();
                return;
            }
        }
    }
}

void MainGraph::on_selection_changed_global(){
    bool foundVLine = false;
    selected.clear();
    for(auto iter = lbldData.begin(); iter!= lbldData.end(); iter++)
    {
        if(iter->line->selected())
        {
            selected.push_back(iter->t);
            foundVLine = true;
        }
    }
    if(!foundVLine)
    {
        QCPDataSelection dataSelection = plt->graph(0)->selection();
        if(!dataSelection.isEmpty())
        {
            int numPoint = 0;
            foreach (QCPDataRange dataRange, dataSelection.dataRanges())
            {
              QCPGraphDataContainer::const_iterator begin = plt->graph(0)->data()->at(dataRange.begin()); // get range begin iterator from index
              QCPGraphDataContainer::const_iterator end = plt->graph(0)->data()->at(dataRange.end()); // get range begin iterator from index

              for (auto iter=begin; iter != end; iter++)
              {
                  selected.push_back(iter->key);
                  numPoint++;
              }
            }
            if (numPoint == 1 && !isMultiSelection && !keyLbl.empty()){
                addToSaveList(keyLbl[nextLblPos]);
                nextLblPos++;
                if(nextLblPos >= keyLbl.size())
                    nextLblPos = 0;
                QString nextLblStr = "Next label: ";
                nextLabel->setText(nextLblStr.append(keyLbl[nextLblPos].label));
            }
        }
    }
}

void MainGraph::addToSaveList(Label inLbl){
    bool flagPresent;
    QString s;
    std::list<int> diff;
    for (auto iterNew = selected.begin(); iterNew != selected.end(); iterNew++)
    {
        // *iterNew is just an int --> times
        flagPresent = false;
        for(auto iterPresent = lbldData.begin(); iterPresent != lbldData.end(); iterPresent++)
        {
            if (iterPresent->t==*iterNew && iterPresent->lbl.key == inLbl.key){
                flagPresent = true;
            }
        }
        if(!flagPresent){
            newSaveFile = true;
            LabelData thisPt = {inLbl,*iterNew,new VLine(plt,ColorTheme::Red)};
            addLabelDataToPlot(thisPt);
            lbldData.push_back(thisPt);
            s = "Labeld (Unsaved changes)";
        }
    }
    if(flagPresent)
        s = "Label(s) already present";
    ui->ev->setText(s);
    plt->deselectAll();
    selected.clear();
    plt->replot();
}

void MainGraph::addLabelDataToPlot(LabelData lblToAdd){
    //Get value
    //double val = plt->graph(0)->data()->findEnd(lblToAdd.t-1,false)->value;
    double val = findAvgLastNPoints(lblToAdd.t);
    lblToAdd.line->start->setCoords(lblToAdd.t, val+400);
    lblToAdd.line->end->setCoords(lblToAdd.t, val-100);

    lblToAdd.line->UpdateLabel(lblToAdd.t, val+425,QString(lblToAdd.lbl.key));
    lblToAdd.line->SetVisibleCustom(true);
}

void MainGraph::removeFromSaveList(){//problem
    QString s;
    for (auto iterNew = selected.begin(); iterNew != selected.end(); iterNew++)
    {
        for(auto iterPresent = lbldData.begin(); iterPresent != lbldData.end(); iterPresent++)
        {
            if (iterPresent->t==*iterNew){
                plt->removeItem(iterPresent->line);
                lbldData.erase(iterPresent--);//erase, after position the iterator to the element before, next step it will go to the correct element
                newSaveFile = true;
                s = "Removed (Unsaved changes)";
            }
        }
    }
    if(selected.size()>0 and s.isEmpty())
        s = "Nothing to erase";
    ui->ev->setText(s);
    plt->deselectAll();
    selected.clear();
    plt->replot();
}

void MainGraph::on_item_click(QCPAbstractItem * item, QMouseEvent *e){
    if(QCPItemLine *a = qobject_cast<QCPItemLine *>(item)){
        return;
    }
}

void MainGraph::on_mouse_move(QMouseEvent* e){
    double x = plt->xAxis->pixelToCoord(e->localPos().x());
    double y = plt->yAxis->pixelToCoord(e->localPos().y());
    if(!dragging && !e->buttons().testFlag(Qt::LeftButton)){
        double tollerance = 5;
        bool found = false;
        for(auto iterPresent = lbldData.begin(); iterPresent != lbldData.end(); iterPresent++)
        {
            if(std::abs(iterPresent->t-x)<tollerance && iterPresent->line->start->coords().y()>y && iterPresent->line->end->coords().y()<y ){
                QPen pen;
                pen.setBrush(Qt::white);
                pen.setWidth(5);
                pen.setColor(QColor(255, 50, 10));
                iterPresent->line->setPen(pen);
                hovered = &(*iterPresent);
                found = true;
            }
            else{
                QPen pen;
                pen.setBrush(Qt::white);
                pen.setWidth(2);
                pen.setColor(QColor(255, 50, 10));
                iterPresent->line->setPen(pen);
            }
        }
        if(!found)
            hovered = nullptr;
        plt->replot();
    }
    else if(dragging && e->buttons().testFlag(Qt::LeftButton)){
        double xNew;
        double xNew1 = plt->graph(0)->data()->findBegin(x,true)->key;
        double xNew2 = plt->graph(0)->data()->findEnd(x-1,false)->key;
        if(std::abs(x-xNew1)<std::abs(x-xNew2))
            xNew = xNew1;
        else
            xNew = xNew2;
        double vNew = findAvgLastNPoints(xNew);

        hovered->line->start->setCoords(xNew, vNew+400);
        hovered->line->end->setCoords(xNew, vNew-100);

        hovered->line->UpdateLabel(xNew, vNew+425,QString(hovered->lbl.key));
        hovered->line->SetVisibleCustom(true);

        hovered->t = static_cast<unsigned long>(xNew);
        plt->replot();
    }
}

void MainGraph::on_QCPMouseReleased_release(QMouseEvent*e){
    if (dragging){
        dragging = false;
        hovered = nullptr;
        plt->setInteraction(QCP::iRangeDrag, true);
        newSaveFile = true;
        QString s = "Moved (Unsaved changes)";
        ui->ev->setText(s);
    }
}

double MainGraph::findAvgLastNPoints(double key, unsigned long pointNumber){
    double sum = 0;
    double prevKey = plt->graph(0)->data()->findBegin(key+1,true)->key;
    double prevVal;
    for(unsigned long i = 0; i<pointNumber; i++)
    {
        prevVal = plt->graph(0)->data()->findBegin(prevKey,true)->value;
        sum += prevVal;
        prevKey = plt->graph(0)->data()->findBegin(prevKey,true)->key;
    }
    return sum/double(pointNumber);
}
