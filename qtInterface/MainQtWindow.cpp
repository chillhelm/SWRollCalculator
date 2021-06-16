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

#include <QString>

#include "MainQtWindow.h"

MainQtWindow::MainQtWindow(QWidget* parent_): QWidget(parent_), nRCWCount(0) {
    rcw1 = new RollCompositionWidget;
    chart = new QtCharts::QChart();
    chart->setTitle("Probabilities of Success and Failure");
    chart->setAnimationOptions(QtCharts::QChart::SeriesAnimations);
    QStringList categories;
    categories << "Crit. Fail" << "Fail" << "Success" << "S+1 Raise" << "S+2 Raise" << "S+3Raise"<<"S+>4 Raise";

    axisX = new QtCharts::QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);

    axisY = new QtCharts::QValueAxis();

    chart->addAxis(axisY, Qt::AlignLeft);

    chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    VBoxLayout = new QVBoxLayout(this);

    RollSetupRow = new QWidget(this);
    HBoxLayout = new QHBoxLayout(RollSetupRow);
    HBoxLayout->addWidget(rcw1);

    QPushButton *addRCWButton = new QPushButton("Add Roll", RollSetupRow);
    QPushButton *plotButton = new QPushButton("Plot", RollSetupRow);
    QObject::connect(plotButton, QOverload<bool>::of(&QPushButton::clicked), [this](bool){this->updateChart();});
    QObject::connect(addRCWButton, QOverload<bool>::of(&QPushButton::clicked), [this](bool){this->addRCW();});
    plotButton->resize(120,120);
    HBoxLayout->addWidget(addRCWButton);
    HBoxLayout->addWidget(plotButton);

    RollSetupRow->setMinimumHeight(150);

    VBoxLayout->addWidget(RollSetupRow);
    VBoxLayout->addWidget(chartView);

    updateChart();

}

MainQtWindow::~MainQtWindow(void) {
    delete rcw1;
    delete chart;
    delete axisX;
    delete axisY;
    delete chartView;

    delete VBoxLayout;
    delete HBoxLayout;
    delete RollSetupRow;
}

double MainQtWindow::fillBarSetFromStochasticObject(QtCharts::QBarSet& set, const std::shared_ptr<StochasticObject>& pStochasticObject) {
    double max = -std::numeric_limits<double>::infinity();
    set.remove(0, set.count());
    for(double x=-1.;x<5;++x) {
        double p =100.*(pStochasticObject->distributionFunction(x) - pStochasticObject->distributionFunction(x-1.));
        set << p;
        max = std::max(p,max);
    }
    double p =100.*(1.-pStochasticObject->distributionFunction(3.));
    max = std::max(p,max);
    set << p;
    return max;
}

void MainQtWindow::updateChart(void) {
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
    if(max<0.) max = 1.;
    auto y_lim = std::ceil(max/10.)*10.;
    axisY->setRange(0.,y_lim);
    chart->addSeries(series);

    series->attachAxis(axisX);
    series->attachAxis(axisY);
}

void MainQtWindow::addRCW(void) {
    auto rcw = std::make_unique<RollCompositionWidget>();
    ++nRCWCount;
    HBoxLayout->insertWidget(nRCWCount,rcw.release());
}