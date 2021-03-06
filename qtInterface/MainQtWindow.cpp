/*
Copyright 2021 Wilhelm Neubert
This file is part of SW Roll Calculator.

SW Roll Calculator is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SW Roll Calculator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SW Roll Calculator.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

#include <QString>
#include <QPen>
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <QMenuBar>

#include "MainQtWindow.h"

MainQtWindow::MainQtWindow(QWidget* parent_): QWidget(parent_), nRCWCount(0), nPlotRaiseNumber(4), bDisplayExactProbabilities(true), optionsWindow(new OptionsMenu(*this, nullptr)) {
    chart = new QtCharts::QChart();
    chart->setTitle("Probabilities of Success and Failure");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
    QStringList categories;
    categories << "Crit. Fail" << "(Non-Crit) Fail" << "Success" << "S+1 Raise" << "S+2 Raise" << "S+3Raise" << "S+4Raise"<<"S+>4 Raise";

    axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QtCharts::QValueAxis();
    QPen penYAxisLines;
    penYAxisLines.setStyle(Qt::DashLine);
    penYAxisLines.setBrush(Qt::lightGray);

    axisY->setGridLinePen(penYAxisLines);

    chart->addAxis(axisY, Qt::AlignLeft);
    axisY->setTickCount(10);

    chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    VBoxLayout = new QVBoxLayout(this);

    RollSetupRow = new QWidget(this);
    HBoxLayout = new QHBoxLayout(RollSetupRow);

    QWidget *buttonBox = new QWidget(RollSetupRow);
    buttonBox->setMinimumWidth(150);
    QVBoxLayout *buttonBoxLayout = new QVBoxLayout(buttonBox);

    QPushButton *addRCWButton = new QPushButton("Add Roll", buttonBox);
    QPushButton *plotButton = new QPushButton("Plot", buttonBox);
    QObject::connect(plotButton, QOverload<bool>::of(&QPushButton::clicked), [this](bool){this->updateChart();});
    QObject::connect(addRCWButton, QOverload<bool>::of(&QPushButton::clicked), [this](bool){this->addRCW();});
    buttonBoxLayout->addWidget(addRCWButton);
    buttonBoxLayout->addWidget(plotButton);
    plotButton->resize(80,20);
    addRCWButton->resize(80,20);

    QWidget *saveButtonBox = new QWidget(buttonBox);
    QHBoxLayout *saveButtonsLayout = new QHBoxLayout(saveButtonBox);
    QPushButton *exportCSVButton = new QPushButton("Export CSV", saveButtonBox);
    QPushButton *exportPNGButton = new QPushButton("Save Plot", saveButtonBox);
    QObject::connect(exportCSVButton, QOverload<bool>::of(&QPushButton::clicked), [this](bool){this->exportCSV();});
    QObject::connect(exportPNGButton, QOverload<bool>::of(&QPushButton::clicked), [this](bool){this->exportPNG();});
    saveButtonsLayout->addWidget(exportCSVButton);
    saveButtonsLayout->addWidget(exportPNGButton);
    buttonBoxLayout->addWidget(saveButtonBox);

    HBoxLayout->addStretch(0);
    HBoxLayout->addWidget(buttonBox);
    buttonBox->resize(80,150);
    RollSetupRow->setMinimumHeight(150);

    VBoxLayout->addWidget(RollSetupRow);
    VBoxLayout->addWidget(chartView);

    addRCW();
    updateChart();

    createMenuBar();

    setWindowTitle("SW Roll Calculator");
    setWhatsThis("SW Roll Calculator");
}

MainQtWindow::~MainQtWindow(void) {

}

double MainQtWindow::fillBarSetFromStochasticObject(QtCharts::QBarSet& set, const std::shared_ptr<StochasticObject>& pStochasticObject) {
    double max = -std::numeric_limits<double>::infinity();
    set.remove(0, set.count());
    for(double x=-1.;x<nPlotRaiseNumber+2;++x) {
        double p = .0;
        if(bDisplayExactProbabilities || x<1.)
            p = 100.*(pStochasticObject->distributionFunction(x) - pStochasticObject->distributionFunction(x-1.));
        else
            p = 100.*(1. - pStochasticObject->distributionFunction(x-1.));
        set << p;
        max = std::max(p,max);
    }
    if(bDisplayExactProbabilities) {
        double p =100.*(1.-pStochasticObject->distributionFunction(nPlotRaiseNumber+1.));
        max = std::max(p,max);
        set << p;
    }
    return max;
}

void MainQtWindow::updateChart(void) {
    QStringList categories;
    categories << "Critical Fail"<<"(Non-Crit) Fail"<<(bDisplayExactProbabilities?"Success":">=Success");
    for (int i=1; i<nPlotRaiseNumber+1; ++i) {
        std::stringstream s;
        s<<"Success + "<<(bDisplayExactProbabilities?"":">=")<<i<<(i==1?" Raise":" Raises");
        categories<<s.str().c_str();
    }
    if(bDisplayExactProbabilities) {
        std::stringstream s;
        s<<"Success + >="<<nPlotRaiseNumber+1<<" Raises";
        categories<<s.str().c_str();
    }
    delete axisX;
    axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->removeAllSeries();
    auto series = new QtCharts::QBarSeries;
    auto count = 1;
    double max = -std::numeric_limits<double>::infinity();
    for (auto rcw :RollSetupRow->findChildren<RollCompositionWidget*>()){
        auto set0 = std::make_unique<QtCharts::QBarSet>(QString("Roll ")+QString::number(count));
        max = std::max(max,fillBarSetFromStochasticObject(*set0, rcw->getRoll()));
        series->append(set0.release());
        ++count;
    }
    QObject::connect(series, QOverload<bool,int,QtCharts::QBarSet*>::of(&QtCharts::QBarSeries::hovered), 
            [this](bool status, int index, QtCharts::QBarSet* barset){this->hoveredBar(status, index, barset);});
    if(max<0.) max = 1.;
    auto y_lim = std::ceil(max/10.)*10.;
    axisY->setRange(0.,y_lim);
    chart->addSeries(series);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

}

void MainQtWindow::addRCW(void) {
    auto rcw = std::make_unique<RollCompositionWidget>();
    HBoxLayout->insertWidget(nRCWCount,rcw.release());
    auto newRCW = (RollCompositionWidget*)HBoxLayout->itemAt(nRCWCount)->widget();
    newRCW->setMinimumWidth(150);
    auto deleteItButton = newRCW->getDeleteMeButton();
    QObject::connect(deleteItButton, QOverload<bool>::of(&QPushButton::clicked), [newRCW,this](bool){--(this->nRCWCount); delete newRCW;});

    HBoxLayout->invalidate();
    ++nRCWCount;
}

void MainQtWindow::hoveredBar(bool status, int index, QtCharts::QBarSet* set) {
    if(status) { // On
    } else { // Off
    }
}

void MainQtWindow::exportCSV(void) {
    updateChart();
    QFileDialog saveFileDialog(this, tr("Export as CSV"),"",tr("CSV File (*.csv);;All Files (*)"));
    saveFileDialog.setDefaultSuffix(".csv");
    saveFileDialog.setFileMode(QFileDialog::AnyFile);
    QString filename;
    saveFileDialog.exec();
    auto fileNames = saveFileDialog.selectedFiles();
    if(fileNames.size()<1)
        return;
    filename = fileNames[0];
    if(filename=="")
        return;
    std::ofstream fsCSVFile(filename.toStdString().c_str());
    fsCSVFile << "Result, "<<"Critical Fail, "<<"(Non-Crit) Fail, "<<(bDisplayExactProbabilities?"":">=")<<"Success";
    for (int i=1; i<nPlotRaiseNumber+1; ++i) {
        fsCSVFile<<", Success + "<<(bDisplayExactProbabilities?"":">=")<<i<<(i==1?" Raise":" Raises");
    }
    if(bDisplayExactProbabilities)
        fsCSVFile<<", Success + >="<<nPlotRaiseNumber+1<<" Raises" <<std::endl;
    else
        fsCSVFile<<std::endl;
    int rollIndex=1;
    for (auto rcw :RollSetupRow->findChildren<RollCompositionWidget*>()){
        auto roll = rcw->getRoll();
        fsCSVFile<<"Roll "<<rollIndex<<", ";
        for(double x=-1.;x<nPlotRaiseNumber+2;++x) {
            double p = .0;
            if(bDisplayExactProbabilities || x<1.)
                p = 100.*(roll->distributionFunction(x) - roll->distributionFunction(x-1.));
            else
                p = 100.*(1. - roll->distributionFunction(x-1.));
            fsCSVFile << p<<", ";
        }
        if(bDisplayExactProbabilities) {
            double p =100.*(1.-roll->distributionFunction(nPlotRaiseNumber));
            fsCSVFile<<p;
        }
        fsCSVFile<<std::endl;
    }
}

void MainQtWindow::exportPNG(void) {
    updateChart();
    QFileDialog saveFileDialog(this, tr("Save Plot to PNG"),"",tr("PNG Images (*.png);;All Files (*)"));
    saveFileDialog.setDefaultSuffix(".png");
    saveFileDialog.setFileMode(QFileDialog::AnyFile);
    QString filename;
    saveFileDialog.exec();
    auto fileNames = saveFileDialog.selectedFiles();
    if(fileNames.size()<1)
        return;
    filename = fileNames[0];
    if(filename=="")
        return;
    if (!filename.endsWith(".png"))
        filename+=".png";
    QImage image(chartView->contentsRect().size(),QImage::Format_ARGB32);
    QPainter p(&image);
    chartView->render(&p);
    image.save(filename, "PNG");
}

void MainQtWindow::createMenuBar(void) {
    QMenuBar *menuBar = new QMenuBar(this);
    QMenu *main = new QMenu("Menu");
    menuBar->addMenu(main);
    this->layout()->setMenuBar(menuBar);
    auto aboutAction = main->addAction("About");
    QObject::connect(aboutAction, QOverload<bool>::of(&QAction::triggered), [this](bool){InfoWindow *iw = new InfoWindow; iw->show();});
    auto optionsAction = main->addAction("Options");
    QObject::connect(optionsAction, QOverload<bool>::of(&QAction::triggered), [this](bool){optionsWindow->show();});
    auto closeAction = main->addAction("Close");
    QObject::connect(closeAction, QOverload<bool>::of(&QAction::triggered), [this](bool){this->close();});
}

void MainQtWindow::setPlotRaiseNumber(int nPlotRaiseNumber_) {
    nPlotRaiseNumber = nPlotRaiseNumber_;
    updateChart();
}

void MainQtWindow::setDisplayExactProbabilities(bool bToggle_) {
    bDisplayExactProbabilities = bToggle_;
    updateChart();
}

