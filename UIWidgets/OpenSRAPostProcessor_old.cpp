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

#include "CSVReaderWriter.h"
#include "AssetInputWidget.h"
#include "GeneralInformationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "OpenSRAPostProcessor.h"
#include "REmpiricalProbabilityDistribution.h"
#include "TablePrinter.h"
#include "TreeItem.h"
#include "QGISVisualizationWidget.h"
#include "ComponentDatabaseManager.h"
#include "WorkflowAppOpenSRA.h"
#include "EmbeddedMapViewWidget.h"
#include "MutuallyExclusiveListWidget.h"

#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QFileInfo>
#include <QFontMetrics>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineSeries>
#include <QPixmap>
#include <QListView>
#include <QPrinter>
#include <QSplitter>
#include <QStackedBarSeries>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextCursor>
#include <QTextTable>
#include <QValueAxis>
#include <QToolButton>
#include <QPushButton>

// GIS headers
#include <qgslinesymbol.h>
#include <qgsmapcanvas.h>
#include <qgsrenderer.h>
#include <qgsgraduatedsymbolrenderer.h>
#include <qgslayertreeview.h>

using namespace QtCharts;

OpenSRAPostProcessor::OpenSRAPostProcessor(QWidget *parent, QGISVisualizationWidget* visWidget) : SimCenterAppWidget(parent), theVisualizationWidget(visWidget)
{
    PGVTreeItem = nullptr;
    PGDTreeItem = nullptr;
    totalTreeItem = nullptr;
    defaultItem = nullptr;
    thePipelineDb = nullptr;
    // The first n columns is information about the component and not results
    numInfoCols = 8;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Create a view menu for the dockable windows
    mainWidget = new QSplitter();
    mainWidget->setOrientation(Qt::Horizontal);
    mainWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    listWidget = new MutuallyExclusiveListWidget(this, "Results");

    connect(listWidget, &MutuallyExclusiveListWidget::itemChecked, this, &OpenSRAPostProcessor::handleListSelection);
    connect(listWidget, &MutuallyExclusiveListWidget::clearAll, this, &OpenSRAPostProcessor::clearAll);
    connect(theVisualizationWidget,&VisualizationWidget::emitScreenshot,this,&OpenSRAPostProcessor::assemblePDF);

    listWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    // Create the table that will show the Component information
    tableWidget = new QWidget(this);

    PGVResultsTableWidget = new QTableWidget(this);
    PGVResultsTableWidget->verticalHeader()->setVisible(false);
    PGVResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    PGVResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    PGVResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    PGVResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    PGVResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    PGVResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    PGVResultsTableWidget->setVisible(false);

    PGDResultsTableWidget = new QTableWidget(this);
    PGDResultsTableWidget->verticalHeader()->setVisible(false);
    PGDResultsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    PGDResultsTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);
    PGDResultsTableWidget->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    PGDResultsTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    PGDResultsTableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    PGDResultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    PGDResultsTableWidget->setVisible(false);

    // Get the map view widget
    auto mapView = theVisualizationWidget->getMapViewWidget("ResultsWidget");
    mapViewSubWidget = std::unique_ptr<SimCenterMapcanvasWidget>(mapView);

    // Enable the selection tool
    mapViewSubWidget->enableSelectionTool();

    // Popup stuff
    // Once map is set, connect to MapQuickView mouse clicked signal
    // connect(mapViewSubWidget.get(), &MapViewSubWidget::mouseClick, theVisualizationWidget, &VisualizationWidget::onMouseClickedGlobal);

    mainWidget->addWidget(mapViewSubWidget.get());

    QWidget* rightHandWidget = new QWidget();
    QVBoxLayout* rightHandLayout = new QVBoxLayout(rightHandWidget);
    rightHandLayout->setMargin(0);
    rightHandLayout->setContentsMargins(0, 0, 0, 0);

    rightHandLayout->addWidget(listWidget);

    QPushButton* modifyLegendButton = new QPushButton("Modify Legend",this);
    connect(modifyLegendButton, &QPushButton::clicked ,this, &OpenSRAPostProcessor::handleModifyLegend);

    rightHandLayout->addWidget(modifyLegendButton);

    //    auto legView = theVisualizationWidget->getLegendView();
    //    if(legView != nullptr)
    //    {
    //        QLabel* legLabel = new QLabel("Legend",this);
    //        rightHandLayout->addWidget(legLabel);
    //        rightHandLayout->addWidget(legView);
    //    }

    mainWidget->addWidget(rightHandWidget);

    mainLayout->addWidget(mainWidget);

    // The number of header rows in the Pelicun results file
    numHeaderRows = 1;

    mainWidget->setStretchFactor(0,2);

    // Now add the splitter handle
    // Note: index 0 handle is always hidden, index 1 is between the two widgets
    QSplitterHandle *handle = mainWidget->handle(1);

    if(handle == nullptr)
    {
        qDebug()<<"Error getting the handle";
        return;
    }

    auto buttonHandle = new QToolButton(handle);
    QVBoxLayout *layout = new QVBoxLayout(handle);
    layout->setSpacing(0);
    layout->setMargin(0);

    mainWidget->setHandleWidth(15);

    buttonHandle->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    buttonHandle->setDown(false);
    buttonHandle->setAutoRaise(false);
    buttonHandle->setCheckable(false);
    buttonHandle->setArrowType(Qt::LeftArrow);
    buttonHandle->setStyleSheet("QToolButton{border:0px solid}; QToolButton:pressed {border:0px solid}");
    buttonHandle->setIconSize(buttonHandle->size());
    layout->addWidget(buttonHandle);

}


void OpenSRAPostProcessor::handleModifyLegend(void)
{

//    auto pipelineInputWidget = theVisualizationWidget->getComponentWidget("GASPIPELINES");

//    if(pipelineInputWidget == nullptr)
//        return;

//    auto pipelineLayer = pipelineInputWidget->getSelectedFeatureLayer();

//    if(pipelineLayer == nullptr)
//        return;

//    auto layerId = pipelineLayer->layerId();

//    theVisualizationWidget->handlePlotColorChange(layerId);
}


void OpenSRAPostProcessor::showEvent(QShowEvent *e)
{
    auto mainCanvas = mapViewSubWidget->getMainCanvas();

    auto mainExtent = mainCanvas->extent();

    mapViewSubWidget->mapCanvas()->zoomToFeatureExtent(mainExtent);
    QWidget::showEvent(e);
}


int OpenSRAPostProcessor::importResultVisuals(const QString& pathToResults)
{

    QDir resultsDir(pathToResults);

    // Get the existing files in the folder
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

    if(existingCSVFiles.empty())
    {
        errorMessage("The results folder is empty. Did you include DV's in the analysis?");
        return -1;
    }

    for(auto&& it : existingCSVFiles)
    {
        QString pathToFile = pathToResults+ QDir::separator() + it;

        if(it.startsWith("ScenarioTraces"))
        {
            this->importScenarioTraces(pathToFile);
        }
        else if(it.startsWith("FaultCrossings"))
        {
            this->importFaultCrossings(pathToFile);
        }
    }

    return 0;
}


int OpenSRAPostProcessor::importScenarioTraces(const QString& pathToFile)
{

    if(pathToFile.isEmpty())
        return 0;

    CSVReaderWriter csvTool;

    QString errMsg;

    auto traces = csvTool.parseCSVFile(pathToFile, errMsg);
    if(!errMsg.isEmpty())
    {
        errorMessage(errMsg);
        return -1;
    }

    if(traces.size() < 2)
    {
        statusMessage("No fault traces available.");
        return 0;
    }

    QgsFields featFields;
    featFields.append(QgsField("AssetType", QVariant::String));
    featFields.append(QgsField("TabName", QVariant::String));
    featFields.append(QgsField("SourceIndex", QVariant::String));

    // Create the pipelines layer
    auto mainLayer = theVisualizationWidget->addVectorLayer("linestring","Scenario Faults");

    if(mainLayer == nullptr)
    {
        this->errorMessage("Error adding a vector layer");
        return -1;
    }

    QList<QgsField> attribFields;
    for(int i = 0; i<featFields.size(); ++i)
        attribFields.push_back(featFields[i]);

    auto pr = mainLayer->dataProvider();

    mainLayer->startEditing();

    auto res = pr->addAttributes(attribFields);

    if(!res)
        this->errorMessage("Error adding attributes to the layer" + mainLayer->name());

    mainLayer->updateFields(); // tell the vector layer to fetch changes from the provider

    auto headers = traces.front();

    auto indexListofTraces = headers.indexOf("ListOfTraces");

    traces.pop_front();

    auto numAtrb = attribFields.size();

    for(auto&& it : traces)
    {
        auto coordString = it.at(indexListofTraces);

        auto geometry = theVisualizationWidget->getMultilineStringGeometryFromJson(coordString);

        if(geometry.isEmpty())
        {
            QString msg ="Error getting the feature geometry for scenario faults layer";
            this->errorMessage(msg);

            return -1;
        }

        // create the feature attributes
        QgsAttributes featureAttributes(numAtrb);

        featureAttributes[0] = QVariant("SCENARIO_TRACES");
        featureAttributes[1] = QVariant("Scenario Traces");
        featureAttributes[2] = QVariant(it.at(0));


        QgsFeature feature;
        feature.setFields(featFields);

        feature.setGeometry(geometry);

        feature.setAttributes(featureAttributes);

        if(!feature.isValid())
            return -1;

        auto res = pr->addFeature(feature, QgsFeatureSink::FastInsert);
        if(!res)
        {
            this->errorMessage("Error adding the feature to the layer");
            return -1;
        }
    }

    mainLayer->commitChanges(true);
    mainLayer->updateExtents();

    QgsLineSymbol* markerSymbol = new QgsLineSymbol();

    QColor featureColor = QColor(0,0,0,200);
    auto weight = 1.0;

    markerSymbol->setWidth(weight);
    markerSymbol->setColor(featureColor);
    theVisualizationWidget->createSimpleRenderer(markerSymbol,mainLayer);


    return 0;
}

int OpenSRAPostProcessor::importFaultCrossings(const QString& pathToFile)
{

    return 0;
}



void OpenSRAPostProcessor::importResults(const QString& pathToResults)
{
    qDebug() << "OpenSRAPostProcessor: " << pathToResults;

    QString errMsg;


    // Get the pipelines database
    thePipelineDb = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

    if(thePipelineDb == nullptr)
    {
        errMsg = "Could not get the pipeline database";
        throw errMsg;
    }

    QString pathToDv = pathToResults + QDir::separator() + "DV";

    try {
        this->importDVresults(pathToDv);
    }
    catch (QString err) {
        errorMessage(err);
    }


    QString pathToScenarioTraces = pathToResults + QDir::separator() + "IM" + QDir::separator() + "SeismicSource";
    this->importResultVisuals(pathToScenarioTraces);


    listWidget->expandAll();
}


int OpenSRAPostProcessor::importDVresults(const QString& pathToResults)
{

    QDir resultsDir(pathToResults);

    QString errMsg;

    // Get the existing files in the folder
    QStringList acceptableFileExtensions = {"*.csv"};
    QStringList existingCSVFiles = resultsDir.entryList(acceptableFileExtensions, QDir::Files);

    if(existingCSVFiles.empty())
    {
        errMsg = "The results folder is empty. Did you include DV's in the analysis?";
        throw errMsg;
    }

    QString PGDResultsSheet;
    QString PGVResultsSheet;
    QString AllResultsSheet;

    for(auto&& it : existingCSVFiles)
    {
        if(it.startsWith("RepairRatePGD"))
            PGDResultsSheet = it;
        else if(it.startsWith("RepairRatePGV"))
            PGVResultsSheet = it;
        else if(it.startsWith("AllResults"))
            AllResultsSheet = it;
    }

    // Create the CSV reader/writer tool
    CSVReaderWriter csvTool;

    // Vector to hold the attributes
    QVector< QgsAttributes > fieldAttributes;
    QStringList fieldNames;

    if(!PGVResultsSheet.isEmpty())
    {
        RepairRatePGV = csvTool.parseCSVFile(pathToResults + QDir::separator() + PGVResultsSheet,errMsg);
        if(!errMsg.isEmpty())
            throw errMsg;

        if(!RepairRatePGV.empty())
            this->processPGVResults(RepairRatePGV,fieldNames,fieldAttributes);
        else
        {
            errMsg = "The PGV results are empty";
            throw errMsg;
        }
    }

    if(!PGDResultsSheet.isEmpty())
    {
        RepairRatePGD = csvTool.parseCSVFile(pathToResults + QDir::separator() + PGDResultsSheet,errMsg);
        if(!errMsg.isEmpty())
            throw errMsg;

        if(!RepairRatePGD.empty())
            this->processPGDResults(RepairRatePGD,fieldNames,fieldAttributes);
        else
        {
            errMsg = "The PGD results are empty";
            throw errMsg;
        }
    }

    if(!AllResultsSheet.isEmpty())
    {
        RepairRateAll = csvTool.parseCSVFile(pathToResults + QDir::separator() + AllResultsSheet,errMsg);
        if(!errMsg.isEmpty())
            throw errMsg;

        if(!RepairRateAll.empty())
            this->processTotalResults(RepairRateAll,fieldNames,fieldAttributes);
        else
        {
            errMsg = "The total results are empty";
            throw errMsg;
        }
    }

    // Get the pipelines database

    auto thePipelineDB = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

    if(thePipelineDB == nullptr)
    {
        QString msg = "Error getting the pipeline database from the input widget!";
        throw msg;
    }

    if(thePipelineDB->isEmpty())
    {
        QString msg = "Pipeline database is empty";
        throw msg;
    }

    auto selFeatLayer = thePipelineDB->getSelectedLayer();
    mapViewSubWidget->setCurrentLayer(selFeatLayer);

//    mapViewSubWidget->addLayerToLegend(selFeatLayer);

    // Starting editing
    thePipelineDB->startEditing();

    auto res = thePipelineDB->addNewComponentAttributes(fieldNames,fieldAttributes,errMsg);
    if(!res)
        throw errMsg;

    // Commit the changes
    thePipelineDB->commitChanges();

    defaultItem->setState(2);

    listWidget->expandAll();

    return 0;
}


int OpenSRAPostProcessor::processPGVResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes)
{

    auto numRows = DVResults.size();

    // Check if there is data in the results, and not just the header rows
    if(numRows < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    auto numHeaderColumns = DVResults.at(0).size();

    if(numHeaderColumns < numInfoCols)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    if(PGVTreeItem == nullptr)
    {
        PGVTreeItem = listWidget->addItem("Shaking Induced");
        PGVTreeItem->setIsCheckable(false);
    }

    QStringList tableHeadings;

    // Add the ID heading
    tableHeadings<<DVResults.at(0).at(0);

    // Add the result headings
    for(int i = numInfoCols; i< numHeaderColumns; ++i)
    {
        QString headerStr =  DVResults.at(0).at(i);

        tableHeadings<<headerStr;

        auto itemStr = headerStr;
        itemStr.remove("PGV_");
        auto item = listWidget->addItem(itemStr,PGVTreeItem);

        // Set the header string as a property so I can find the header value later
        item->setProperty("HeaderString",headerStr);
    }

    PGVResultsTableWidget->setColumnCount(tableHeadings.size());
    PGVResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    PGVResultsTableWidget->setRowCount(DVResults.size()-numHeaderRows);

    QVector< QgsAttributes > attributes(DVResults.size()-numHeaderRows, QgsAttributes(numHeaderColumns-numInfoCols));

    // Start at the row where headers end
    for(int row = numHeaderRows, count = 0; row<numRows; ++row, ++count)
    {
        auto inputRow = DVResults.at(row);

        auto pipelineID = objectToInt(inputRow.at(0));

        // Put the ID item in the table
        auto tableIDItem = new QTableWidgetItem(QString::number(pipelineID));
        PGVResultsTableWidget->setItem(row-1,0, tableIDItem);

        auto& rowData = attributes[count];

        // Populate the table and database with the results
        for(int j = 1, k = numInfoCols; k<numHeaderColumns; ++k, ++j)
        {
            // Add add the result to the table
            auto tableVal = QString(inputRow.at(k));

            auto tableItem = new QTableWidgetItem(tableVal);

            PGVResultsTableWidget->setItem(row-1,j, tableItem);

            // Add the result to the database
            auto value = inputRow.at(k);

            rowData[j-1] = QVariant(value.toDouble());
        }
    }

    // Append the new fields and data

    // We do not need the first column that contains the id
    tableHeadings.pop_front();

    fieldNames.append(tableHeadings);

    fieldAttributes.append(attributes);

    return 0;
}


int OpenSRAPostProcessor::processPGDResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes)
{

    auto numRows = DVResults.size();

    // Check if there is data in the results, and not just the header rows
    if(numRows < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    auto numHeaderColumns = DVResults.at(0).size();

    if(numHeaderColumns < numInfoCols)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    if(PGDTreeItem == nullptr)
    {
        PGDTreeItem = listWidget->addItem("Deformation Induced");
        PGDTreeItem->setIsCheckable(false);
    }

    QStringList tableHeadings;

    // Add the ID heading
    tableHeadings<<DVResults.at(0).at(0);

    // Add the result headings
    for(int i = numInfoCols; i< numHeaderColumns; ++i)
    {
        QString headerStr =  DVResults.at(0).at(i);

        tableHeadings<<headerStr;

        auto itemStr = headerStr;
        itemStr.remove("PGD_");
        auto item = listWidget->addItem(itemStr,PGDTreeItem);

        // Set the header string as a property so I can find the header value later
        item->setProperty("HeaderString",headerStr);
    }

    auto numNewAttributes = numRows-numHeaderRows;

    PGDResultsTableWidget->setColumnCount(tableHeadings.size());
    PGDResultsTableWidget->setHorizontalHeaderLabels(tableHeadings);
    PGDResultsTableWidget->setRowCount(numNewAttributes);

    // Start at the row where headers end  
    for(int row = numHeaderRows, count = 0; row<numRows; ++row, ++count)
    {
        auto inputRow = DVResults.at(row);

        auto pipelineID = objectToInt(inputRow.at(0));


        // Put the ID item in the table
        auto tableIDItem = new QTableWidgetItem(QString::number(pipelineID));
        PGDResultsTableWidget->setItem(row-1,0, tableIDItem);

        auto& rowData = fieldAttributes[count];
        rowData.reserve(rowData.size()+numNewAttributes);

        // Populate the table and database with the results
        for(int j = 1, k = numInfoCols; k<numHeaderColumns; ++k, ++j)
        {
            // Add add the result to the table
            auto tableVal = QString(inputRow.at(k));

            auto tableItem = new QTableWidgetItem(tableVal);

            PGDResultsTableWidget->setItem(row-1,j, tableItem);

            // Add the result to the database
            auto value = inputRow.at(k);

            rowData.push_back(QVariant(value.toDouble()));
        }
    }

    // Append the new fields
    // We do not need the first column that contains the id
    tableHeadings.pop_front();

    fieldNames.append(tableHeadings);

    return 0;
}


int OpenSRAPostProcessor::processTotalResults(const QVector<QStringList>& DVResults, QStringList& fieldNames, QVector<QgsAttributes>& fieldAttributes)
{

    auto numRows = DVResults.size();

    // Check if there is data in the results, and not just the header rows
    if(numRows < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    auto numHeaderColumns = DVResults.at(0).size();

    if(numHeaderColumns < numInfoCols)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    if(totalTreeItem == nullptr)
    {
        totalTreeItem = listWidget->addItem("Total Repair Rates");
        totalTreeItem->setIsCheckable(false);
    }

    QStringList tableHeadings = DVResults.at(0);

    QString headerStr = "TotalRepairRateForAllDemands";

    auto indexOfTotals = tableHeadings.indexOf(headerStr,-1);

    if(indexOfTotals == -1)
    {
        QString msg = "Error getting index to total repairs";
        throw msg;
    }

    QString itemStr = "All demands";

    defaultItem = listWidget->addItem(itemStr,totalTreeItem);

    // Set the header string as a property so I can find the header value later
    defaultItem->setProperty("HeaderString",headerStr);

    // Start at the row where headers end
    for(int row = numHeaderRows, count = 0; row<numRows; ++row, ++count)
    {
        auto inputRow = DVResults.at(row);

        // Add the result to the database
        auto value = inputRow.at(indexOfTotals).toDouble();

        fieldAttributes[count].push_back(QVariant(value));
    }

    fieldNames.append(headerStr);

    return 0;

}


int OpenSRAPostProcessor::printToPDF(const QString& outputPath)
{
    outputFilePath = outputPath;

    theVisualizationWidget->takeScreenShot();

    return 0;
}


void OpenSRAPostProcessor::processResultsSubset(const std::set<int>& selectedComponentIDs)
{

    if(selectedComponentIDs.empty())
        return;

    if(RepairRatePGD.size() < numHeaderRows)
    {
        QString msg = "No results to import!";
        throw msg;
    }

    if(RepairRatePGD.at(numHeaderRows).isEmpty() || RepairRatePGD.last().isEmpty())
    {
        QString msg = "No values in the cells";
        throw msg;
    }

    auto firstID = objectToInt(RepairRatePGD.at(numHeaderRows).at(0));

    auto lastID = objectToInt(RepairRatePGD.last().at(0));

    QVector<QStringList> DVsubset(&RepairRatePGD[0],&RepairRatePGD[numHeaderRows]);

    for(auto&& id : selectedComponentIDs)
    {
        // Check that the ID falls within the bounds of the data
        if(id<firstID || id>lastID)
        {
            QString msg = "ID " + QString::number(id) + " is out of bounds of the results";
            throw msg;
        }

        auto found = false;
        for(int i = numHeaderRows; i<RepairRatePGD.size(); ++i)
        {
            auto inputRow = RepairRatePGD.at(i);

            auto buildingID = objectToInt(inputRow.at(0));

            if(id == buildingID)
            {
                DVsubset << inputRow;
                found = true;
                break;
            }
        }

        if(!found)
        {
            QString msg = "ID " + QString::number(id) + " cannot be found in the results";
            throw msg;
        }
    }

//    this->processPGVResults(DVsubset);
}


int OpenSRAPostProcessor::assemblePDF(QImage screenShot)
{
    // The printer
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::Letter);
    printer.setPageMargins(25.4, 25.4, 25.4, 25.4, QPrinter::Millimeter);
    printer.setFullPage(true);
    qreal leftMargin, topMargin;
    printer.getPageMargins(&leftMargin,&topMargin,nullptr,nullptr,QPrinter::Point);
    printer.setOutputFileName(outputFilePath);

    // Create a new document
    QTextDocument* document = new QTextDocument();
    QTextCursor cursor(document);
    document->setDocumentMargin(25.4);
    document->setDefaultFont(QFont("Helvetica"));

    // Define font styles
    QTextCharFormat normalFormat;
    normalFormat.setFontWeight(QFont::Normal);

    QTextCharFormat titleFormat;
    titleFormat.setFontWeight(QFont::Bold);
    titleFormat.setFontCapitalization(QFont::AllUppercase);
    titleFormat.setFontPointSize(normalFormat.fontPointSize() * 2.0);

    QTextCharFormat captionFormat;
    captionFormat.setFontWeight(QFont::Light);
    captionFormat.setFontPointSize(normalFormat.fontPointSize() / 2.0);
    captionFormat.setFontItalic(true);

    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);

    QFontMetrics normMetrics(normalFormat.font());
    auto lineSpacing = normMetrics.lineSpacing();

    // Define alignment formats
    QTextBlockFormat alignCenter;
    alignCenter.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;
    alignCenter.setAlignment(Qt::AlignCenter);

    QTextBlockFormat alignLeft;
    alignLeft.setAlignment(Qt::AlignLeft);
    alignLeft.setLineHeight(lineSpacing, QTextBlockFormat::LineDistanceHeight) ;

    cursor.movePosition(QTextCursor::Start);

    cursor.insertBlock(alignCenter);

    cursor.insertText("\nOpenSRA Tool\n",titleFormat);

    cursor.insertText("Results Summary\n",boldFormat);

    cursor.movePosition( QTextCursor::End );

    // Ratio of the page width that is printable
    auto useablePageWidth = printer.pageRect(QPrinter::Point).width()-(1.5*leftMargin);

    QRect viewPortRect(0, mapViewSubWidget->height(), mapViewSubWidget->width(), mapViewSubWidget->height());
    QImage cropped = screenShot.copy(viewPortRect);
    document->addResource(QTextDocument::ImageResource,QUrl("Figure1"),cropped);
    QTextImageFormat imageFormatFig1;
    imageFormatFig1.setName("Figure1");
    imageFormatFig1.setQuality(600);
    imageFormatFig1.setWidth(useablePageWidth);

    cursor.setBlockFormat(alignCenter);

    cursor.insertImage(imageFormatFig1);

    cursor.insertText("Regional map visualization.\n",captionFormat);

    cursor.setBlockFormat(alignLeft);

    TablePrinter prettyTablePrinter;
    prettyTablePrinter.printToTable(&cursor, PGVResultsTableWidget,"PGV Results");

    cursor.insertText("\n\n",normalFormat);

    prettyTablePrinter.printToTable(&cursor, PGDResultsTableWidget,"PGD Results");

    document->print(&printer);

    return 0;
}


void OpenSRAPostProcessor::sortTable(int index)
{
    if(index == 0)
        PGVResultsTableWidget->sortByColumn(index,Qt::AscendingOrder);
    else
        PGVResultsTableWidget->sortByColumn(index,Qt::DescendingOrder);

}


void OpenSRAPostProcessor::clear(void)
{
    RepairRatePGV.clear();
    RepairRatePGD.clear();
    RepairRateAll.clear();
    outputFilePath.clear();
    if(thePipelineDb)
        thePipelineDb->clear();
    PGVResultsTableWidget->clear();
    listWidget->clear();

    if(PGVTreeItem != nullptr)
    {
        delete PGVTreeItem;
        PGVTreeItem = nullptr;
    }
    if(PGDTreeItem != nullptr)
    {
        delete PGDTreeItem;
        PGDTreeItem = nullptr;
    }
    if(totalTreeItem != nullptr)
    {
        delete totalTreeItem;
        totalTreeItem = nullptr;
    }
    if(defaultItem != nullptr)
    {
        delete defaultItem;
        defaultItem = nullptr;
    }

    mapViewSubWidget->clear();
}


void OpenSRAPostProcessor::handleListSelection(const TreeItem* itemSelected)
{
    if(itemSelected == nullptr)
        return;

    auto headerString = itemSelected->property("HeaderString").toString();

    if(headerString.isEmpty())
    {
        this->errorMessage("Could not find the property "+headerString+" in item "+itemSelected->getName());
        return;
    }

    // Get the pipelines database
    auto thePipelineDB = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

    if(thePipelineDB == nullptr)
    {
        this->errorMessage("Error getting the pipeline database from the input widget!");
        return;
    }

    if(thePipelineDB->isEmpty())
    {
        this->errorMessage("Pipeline database is empty");
        return;
    }

    auto selFeatLayer = thePipelineDB->getSelectedLayer();
    if(selFeatLayer == nullptr)
    {
        this->errorMessage("Layer is a nullptr in handleListSelection");
        return;
    }

    // Check to see if that field exists in the layer
    auto idx = selFeatLayer->dataProvider()->fieldNameIndex(headerString);

    if(idx == -1)
    {
        this->errorMessage("Could not find the field "+headerString+" in layer "+selFeatLayer->name());
        return;
    }

    auto layerRenderer = selFeatLayer->renderer();
    if(layerRenderer == nullptr)
    {
        this->errorMessage("No layer renderer available in layer "+selFeatLayer->name());
        return;
    }

    // Create a graduated renderer if one does not exist
    if(layerRenderer->type().compare("graduatedSymbol") != 0)
    {
        QVector<QPair<double,double>>classBreaks;
        QVector<QColor> colors;

        classBreaks.append(QPair<double,double>(0.0, 1E-03));
        classBreaks.append(QPair<double,double>(1.00E-03, 1.00E-02));
        classBreaks.append(QPair<double,double>(1.00E-02, 1.00E-01));
        classBreaks.append(QPair<double,double>(1.00E-01, 1.00E+00));
        classBreaks.append(QPair<double,double>(1.00E+00, 1.00E+01));
        classBreaks.append(QPair<double,double>(1.00E+01, 1.00E+10));

        colors.push_back(Qt::darkBlue);
        colors.push_back(QColor(255,255,178));
        colors.push_back(QColor(253,204,92));
        colors.push_back(QColor(253,141,60));
        colors.push_back(QColor(240,59,32));
        colors.push_back(QColor(189,0,38));

        // createCustomClassBreakRenderer(const QString attrName, const QVector<QPair<double,double>>& classBreaks, const QVector<QColor>& colors, QgsVectorLayer * vlayer)
        theVisualizationWidget->createCustomClassBreakRenderer(headerString,selFeatLayer,Qgis::SymbolType::Line,classBreaks,colors);
    }
    else if(auto graduatedRender = dynamic_cast<QgsGraduatedSymbolRenderer*>(layerRenderer))
    {
        graduatedRender->setClassAttribute(headerString);
    }
    else
    {
        this->errorMessage("Unrecognized type of layer renderer available in layer "+selFeatLayer->name());
        return;
    }


    theVisualizationWidget->markDirty();
}


void OpenSRAPostProcessor::clearAll(void)
{

    // Get the pipelines database
    auto thePipelineDB = ComponentDatabaseManager::getInstance()->getAssetDb("GasNetworkPipelines");

    if(thePipelineDB == nullptr)
    {
        this->errorMessage("Error getting the pipeline database from the input widget!");
        return;
    }

    if(thePipelineDB->isEmpty())
    {
        this->errorMessage("Pipeline database is empty");
        return;
    }

    auto selFeatLayer = thePipelineDB->getSelectedLayer();
    if(selFeatLayer == nullptr)
    {
        this->errorMessage("Layer is a nullptr in handleListSelection");
        return;
    }

    QgsLineSymbol* selectedLayerMarkerSymbol = new QgsLineSymbol();

    selectedLayerMarkerSymbol->setWidth(2.0);
    selectedLayerMarkerSymbol->setColor(Qt::darkBlue);
    theVisualizationWidget->createSimpleRenderer(selectedLayerMarkerSymbol,selFeatLayer);

    theVisualizationWidget->markDirty();
}
