#ifndef OpenSRAPostProcessor_H
#define OpenSRAPostProcessor_H
/* *****************************************************************************
Copyright (c) 2016-2021, The Regents of the University of California (Regents).
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.

REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS
PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*************************************************************************** */

// Written by: Stevan Gavrilovic

#include "ComponentDatabase.h"
#include "SimCenterAppWidget.h"
#include "SimCenterMapcanvasWidget.h"

#include <QString>

#include <memory>
#include <set>

class TreeItem;
class REmpiricalProbabilityDistribution;
class QGISVisualizationWidget;
class MutuallyExclusiveListWidget;

class QSplitter;
class QTableWidget;
class QGridLayout;
class QLabel;
class QComboBox;

namespace QtCharts
{
class QChartView;
class QBarSet;
class QChart;
}

namespace Esri
{
namespace ArcGISRuntime
{
class Map;
class MapGraphicsView;
}
}


class OpenSRAPostProcessor : public SimCenterAppWidget
{
    Q_OBJECT

public:

    OpenSRAPostProcessor(QWidget *parent, QGISVisualizationWidget* visWidget);

    void importResults(const QString& pathToResults);

    int importDVresults(const QString& pathToResults);

    int printToPDF(const QString& outputPath);

    // Function to convert a QString and QVariant to double
    // Throws an error exception if conversion fails
    template <typename T>
    auto objectToDouble(T obj)
    {
        // Assume a zero value if the string is empty
        if(obj.isNull())
            return 0.0;

        bool OK;
        auto val = obj.toDouble(&OK);

        if(!OK)
            throw QString("Could not convert the object to a double");

        return val;
    }


    template <typename T>
    auto objectToInt(T obj)
    {
        // Assume a zero value if the string is empty
        if(obj.isNull())
            return 0;

        bool OK;
        auto val = obj.toInt(&OK);

        if(!OK)
            throw QString("Could not convert the object to an integer");

        return val;
    }

    void processResultsSubset(const std::set<int>& selectedComponentIDs);

    void clear(void);

protected:

    void showEvent(QShowEvent *e);

private slots:

    int assemblePDF(QImage screenShot);

    void sortTable(int index);

    void handleListSelection(const TreeItem* itemSelected);

    void clearAll(void);

    void handleModifyLegend(void);

private:

    int processPGVResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes);

    int processPGDResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes);

    int processTotalResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes);

    int importResultVisuals(const QString& pathToResults);
    int importFaultCrossings(const QString& pathToFile);
    int importScenarioTraces(const QString& pathToFile);


    QVector<QStringList> RepairRatePGD;
    QVector<QStringList> RepairRatePGV;
    QVector<QStringList> RepairRateAll;

    TreeItem* PGVTreeItem;
    TreeItem* PGDTreeItem;
    TreeItem* totalTreeItem;

    TreeItem* defaultItem;

    QString outputFilePath;

    QSplitter* mainWidget;

    MutuallyExclusiveListWidget* listWidget;

    QWidget *tableWidget;
    QTableWidget* PGVResultsTableWidget;
    QTableWidget* PGDResultsTableWidget;

    QGISVisualizationWidget* theVisualizationWidget;

    std::unique_ptr<SimCenterMapcanvasWidget> mapViewSubWidget;

    //The number of header rows in the results file
    int numHeaderRows;

    // The number of columns that contain component information
    int numInfoCols;

    ComponentDatabase* thePipelineDb;
};

#endif // OpenSRAPostProcessor_H
