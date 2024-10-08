/* *****************************************************************************
Copyright (c) 2016-2017, The Regents of the University of California (Regents).
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

#include "CustomizedItemModel.h"
#include "GeospatialDataWidget.h"
#include "MultiComponentEDPWidget.h"
#include "MultiComponentDVWidget.h"
#include "MultiComponentDMWidget.h"
#include "RandomVariablesWidget.h"
#include "GeneralInformationWidget.h"
#include "UIWidgets/IntensityMeasureWidget.h"
#include "LocalApplication.h"
#include "PipelineNetworkWidget.h"
#include "RunLocalWidget.h"
#include "RunWidget.h"
#include "UIWidgets/ResultsWidget.h"
#include "OpenSRAComponentSelection.h"
#include "UIWidgets/CustomVisualizationWidget.h"
#include "QGISVisualizationWidget.h"
#include "LineAssetInputWidget.h"
#include "WorkflowAppOpenSRA.h"
#include "UncertaintyQuantificationWidget.h"
#include "MainWindowWorkflowApp.h"
#include "OpenSRAPreferences.h"
#include "LoadResultsDialog.h"
#include "Utils/ProgramOutputDialog.h"
#include "Utils/FileOperations.h"

#include "OpenSRAPreProcessor.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QMenu>
#include <QMenuBar>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QUuid>
#include <QMessageBox>


// static pointer for global procedure set in constructor
static WorkflowAppOpenSRA *theApp = nullptr;

// global procedure
int getNumParallelTasks()
{
    return theApp->getMaxNumParallelTasks();
}


WorkflowAppOpenSRA* WorkflowAppOpenSRA::getInstance()
{
    return theInstance;
}


WorkflowAppOpenSRA *WorkflowAppOpenSRA::theInstance = nullptr;


WorkflowAppOpenSRA::WorkflowAppOpenSRA(QWidget *parent) : WorkflowAppWidget(parent)
{
    // set static pointer for global procedure
    theApp = this;

    theInstance = this;

    resultsDialog = nullptr;

    theWidgetFactory = nullptr;
}


WorkflowAppOpenSRA::~WorkflowAppOpenSRA()
{

}


void WorkflowAppOpenSRA::initialize(void)
{

    QString appDir = OpenSRAPreferences::getInstance()->getAppDir();
    auto backEndFilePath = appDir + QDir::separator();

    // check config.ini to see which version of OpenSRA to run
    auto configIniFilePath = backEndFilePath + "config.ini";
    QString versionToRun = "";
    if(QFile(configIniFilePath).exists())
    {
        QSettings settings(configIniFilePath, QSettings::IniFormat);
        settings.beginGroup("GeneralSettings");
        versionToRun = settings.value("VersionToRun").toString();
        settings.endGroup();
    }
    else
        versionToRun = "Default";

    // check if versionToRun under backend.nda contains methods_param json files
    QString methodParamsDir;
    if (versionToRun == "Default")
        methodParamsDir = backEndFilePath + "methods_params_doc";
    else
        methodParamsDir = backEndFilePath + "nda" + QDir::separator() + versionToRun + QDir::separator() + "methods_params_doc";
    auto defaultMethodParamsDir = backEndFilePath + "methods_params_doc";

    // first try loading from versionToRun path, if can't find file, then load from default path

    // Load the common methods and params json file
    QString commonPath = methodParamsDir + QDir::separator() + "common.json";
    methodsAndParamsObj = getMethodAndParamsObj(commonPath);
    if(methodsAndParamsObj.empty())
    {
        // try default path
        commonPath = defaultMethodParamsDir + QDir::separator() + "common.json";
        methodsAndParamsObj = getMethodAndParamsObj(commonPath, true);
        if(methodsAndParamsObj.empty())
        {
            this->errorMessage("Error loading the methods and params file!");
            return;
        }
    }

    // Load the below ground methods and params json file
    QString belowGroundPath = methodParamsDir + QDir::separator() + "below_ground.json";
    auto belowGroundObj = getMethodAndParamsObj(belowGroundPath);
    if(belowGroundObj.empty())
    {
        // try default path
        belowGroundPath = defaultMethodParamsDir + QDir::separator() + "below_ground.json";
        belowGroundObj = getMethodAndParamsObj(belowGroundPath, true);
        if(belowGroundObj.empty())
        {
            this->errorMessage("Error loading the below ground object file!");
            return;
        }
    }
    methodsAndParamsObj.insert("BelowGround",belowGroundObj);

    // Load the above ground methods and params
    QString aboveGroundPath = methodParamsDir + QDir::separator() + "above_ground.json";
    auto aboveGroundObj = getMethodAndParamsObj(aboveGroundPath);
    if(aboveGroundObj.empty())
    {
        // try default path
        aboveGroundPath = defaultMethodParamsDir + QDir::separator() + "above_ground.json";
        aboveGroundObj = getMethodAndParamsObj(aboveGroundPath, true);
        if(aboveGroundObj.empty())
        {
            this->errorMessage("Error loading the above ground object file!");
            return;
        }
    }
    methodsAndParamsObj.insert("AboveGround",aboveGroundObj);

    // Load the wells and caprocks methods and params
    QString wellsAndCaprocksPath = methodParamsDir + QDir::separator() + "wells_caprocks.json";
    auto wellsCaprocksObj = getMethodAndParamsObj(wellsAndCaprocksPath);
    if(wellsCaprocksObj.empty())
    {
        // try default path
        wellsAndCaprocksPath = defaultMethodParamsDir + QDir::separator() + "wells_caprocks.json";
        wellsCaprocksObj = getMethodAndParamsObj(wellsAndCaprocksPath, true);
        if(wellsCaprocksObj.empty())
        {
            this->errorMessage("Error loading the wells and caprocks object file!");
            return;
        }
    }
    methodsAndParamsObj.insert("WellsCaprocks",wellsCaprocksObj);

    // Create the edit menu with the clear action
    QMenu *editMenu = theMainWindow->menuBar()->addMenu(tr("&Edit"));
    // Set the path to the input file
    editMenu->addAction("Clear GUI Inputs", this, &WorkflowAppOpenSRA::clear);
    editMenu->addAction("Clear Working Directory", this, &WorkflowAppOpenSRA::clearWorkDir);
    editMenu->addAction("Clear Results Directory", this, &WorkflowAppOpenSRA::clearResultsDir);

    // Load the examples
    auto pathToExamplesJson = backEndFilePath + "examples" + QDir::separator() + "Examples.json";

    QFile jsonFile(pathToExamplesJson);
    jsonFile.open(QFile::ReadOnly);
    QJsonDocument exDoc = QJsonDocument::fromJson(jsonFile.readAll());

    auto docObj = exDoc.object();

    // Just add "Examples" to menubar, even if it's empty
    QMenu *exampleMenu = theMainWindow->menuBar()->addMenu(tr("&Examples"));

    /*
    for OpenSRA, look for these 6 example categories:
    - above ground
    - wells caprocks
    - below ground - lateral spread
    - below ground - settlement
    - below ground - landslide
    - below ground - surface fault rupture
    */

    auto keys = docObj.keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        auto key = keys.at(i);

        auto currExObj = docObj.value(key).toObject();
        if(!currExObj.isEmpty())
        {
            QMenu* currExMenu = exampleMenu->addMenu(key);
            auto exContainerObj = currExObj.value("Examples").toArray();
            auto numEx = currExObj.count();
            if(numEx > 0)
            {
                int count = 1;
                for(auto it = exContainerObj.begin(); it!=exContainerObj.end(); ++it)
                {
                    QJsonObject exObj = it->toObject();
                    auto name = exObj.value("name").toString();
//                    auto fullName = "Example " + QString::number(count) + " - " + name;
                    auto fullName = name;
                    auto inputFile = exObj.value("inputFile").toString();
                    // Set the path to the input file
                    auto action = currExMenu->addAction(fullName, this, &WorkflowAppOpenSRA::loadExamples);
                    action->setProperty("InputFile",inputFile);
                    action->setProperty("description",inputFile);
                    action->setProperty("name",fullName);
                    count ++;
                }
            }
        }
    }

    // Results menu to load the results
    QMenu *resultsMenu = new QMenu(tr("&Results"),theMainWindow->menuBar());

    // Set the path to the input file
    resultsMenu->addAction("&Load Results", this, &WorkflowAppOpenSRA::loadResults);
    theMainWindow->menuBar()->addMenu(resultsMenu);


    // Show progress dialog
    QMenu *viewMenu = theMainWindow->menuBar()->addMenu(tr("&View"));
    viewMenu->addAction("Show Status Dialog", this, &WorkflowAppWidget::showOutputDialog);
    viewMenu->addSeparator();

    // Help menu
    theMainWindow->createHelpMenu();

    theVisualizationWidget = new QGISVisualizationWidget(theMainWindow);

    // Create the various widgets
    theRandomVariableWidget = new RandomVariablesWidget();
    thePipelineNetworkWidget = new PipelineNetworkWidget(theVisualizationWidget);

    theWidgetFactory = std::make_unique<WidgetFactory>(thePipelineNetworkWidget->getTheBelowGroundInputWidget());

    theGenInfoWidget = new GeneralInformationWidget();
    //theUQWidget = new UncertaintyQuantificationWidget();
    theIntensityMeasureWidget = new IntensityMeasureWidget(theVisualizationWidget);
    theDamageMeasureWidget = new MultiComponentDMWidget();
    theEDPWidget = new MultiComponentEDPWidget();
    theCustomVisualizationWidget = new CustomVisualizationWidget(theVisualizationWidget);
    theDecisionVariableWidget = new MultiComponentDVWidget();
//    theResultsWidget = new ResultsWidget(nullptr,theVisualizationWidget);
    theGISDataWidget = new GeospatialDataWidget(theVisualizationWidget);

    SimCenterWidget *theWidgets[1];

    localApp = new LocalApplication("OpenSRA.py");
    theRunWidget = new RunWidget(localApp, theWidgets, 0);

    connect(thePipelineNetworkWidget,&MultiComponentR2D::selectionChangedSignal,theEDPWidget,&MultiComponentEDPWidget::handleWidgetSelected);
    connect(thePipelineNetworkWidget,&MultiComponentR2D::selectionChangedSignal,theDamageMeasureWidget,&MultiComponentDMWidget::handleWidgetSelected);
    connect(thePipelineNetworkWidget,&MultiComponentR2D::selectionChangedSignal,theDecisionVariableWidget,&MultiComponentDVWidget::handleWidgetSelected);

    connect(this,SIGNAL(sendInfoMessage(QString)),this,SLOT(infoMessage(QString)));

    // organized by "preprocess" and "run"

    // this is linked to the clicking of the "Preprocessing" button
    connect(localApp, SIGNAL(setupForPreprocessing(QString&,QString&)), this, SLOT(setUpForPreprocessingRun(QString&,QString&)));
    connect(this, SIGNAL(setUpForPreprocessingDone(QString&, QString&)), theRunWidget, SLOT(setupForRunPreprocessingDone(QString&, QString&)));
    connect(localApp, SIGNAL(preprocessingDone()), this, SLOT(preprocessingDone()));

    // this is linked to the clicking of the "Run" button
    connect(localApp, SIGNAL(setupForRun(QString&,QString&)), this, SLOT(setUpForApplicationRun(QString&,QString&)));
    connect(this, SIGNAL(setUpForApplicationRunDone(QString&, QString&)), theRunWidget, SLOT(setupForRunApplicationDone(QString&, QString&)));
    connect(localApp, SIGNAL(processResults(QString, QString, QString)), this, SLOT(postprocessResults(QString, QString, QString)));

    // for left hand side layout
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    this->setLayout(horizontalLayout);
    horizontalLayout->setSpacing(0);
    this->setContentsMargins(0,0,0,0);
    horizontalLayout->setMargin(0);

    // Create the component selection & add the components to it
    theComponentSelection = new OpenSRAComponentSelection(this);
    theComponentSelection->setContentsMargins(0,0,0,0);
    horizontalLayout->addWidget(theComponentSelection);

    theComponentSelection->addComponent(QString("Visualization"), theCustomVisualizationWidget);
    theComponentSelection->addComponent(QString("General\nInformation"), theGenInfoWidget);
    //    theComponentSelection->addComponent(QString("Sampling\nMethod"), theUQWidget);
    theComponentSelection->addComponent(QString("Infrastructure"), thePipelineNetworkWidget);
    theComponentSelection->addComponent(QString("Decision\nVariable"), theDecisionVariableWidget);
    theComponentSelection->addComponent(QString("Damage\nMeasure"), theDamageMeasureWidget);
    theComponentSelection->addComponent(QString("Engineering\nDemand\nParameter"), theEDPWidget);
    theComponentSelection->addComponent(QString("Intensity\nMeasure"), theIntensityMeasureWidget);
    theComponentSelection->addComponent(QString("GIS and\nCPT Data"), theGISDataWidget);
    theComponentSelection->addComponent(QString("Input\nVariables"), theRandomVariableWidget);
//    theComponentSelection->addComponent(QString("Results"), theResultsWidget);

    theComponentSelection->setMinWidth(150);
    theComponentSelection->setMaxWidth(175);
    theComponentSelection->setItemWidthHeight(175,70);

    theComponentSelection->displayComponent("Visualization");

    preprocessor =  new OpenSRAPreProcessor(backEndFilePath, this);

    //    theResultsWidget->processResults("/Users/steve/Desktop/ResToDelete/");

}


QJsonObject WorkflowAppOpenSRA::getMethodAndParamsObj(const QString& path, bool usingDefault)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()){
        if (usingDefault == true)
            this->errorMessage(QString("The methods and params file does not exist! ") + path);
        return QJsonObject();
    }

    QString dirPath = fileInfo.absoluteDir().absolutePath();
    QDir::setCurrent(dirPath);

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        this->errorMessage(QString("Could Not Open File: ") + path);
        return QJsonObject();
    }

    //
    // place contents of file into json object
    //

    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    auto thisMethodsAndParamsObj = doc.object();

    // close file
    file.close();

    if(thisMethodsAndParamsObj.isEmpty())
    {
        this->errorMessage("The methods and parameters file is empty");
        return QJsonObject();
    }

    // Get the human read-able names of the json objects
    std::function<void(const QJsonObject&)> recursiveObj = [&](const QJsonObject& objects){

        QJsonObject::const_iterator objIt;
        for (objIt = objects.begin(); objIt != objects.end(); ++objIt)
        {
            auto obj = objIt.value().toObject();

            auto name = obj["ToDisplay"].toString();
            if(!name.isEmpty())
            {
                auto key = objIt.key();

                if(methodsParamsMap.contains(key))
                {
                    //                    this->errorMessage("Error, the methods and params map already contains the key "+key);
                    //                    auto oldVal = methodsParamsMap.value(key);
                    //                    auto newVal = name;
                    //                    this->errorMessage("Old Val "+oldVal+" new val "+newVal);
                    continue;
                }

                methodsParamsMap.insert(key,name);
            }

            if(!obj.isEmpty())
                recursiveObj(objIt.value().toObject());
        }
    };

    recursiveObj(thisMethodsAndParamsObj);

    return thisMethodsAndParamsObj;
}


void WorkflowAppOpenSRA::loadExamples()
{
    QObject* senderObj = QObject::sender();

    if(senderObj == nullptr)
        return;

    auto appDir = OpenSRAPreferences::getInstance()->getAppDir();
    QString pathToExample = appDir + QDir::separator() + "examples" + QDir::separator();

    pathToExample += QObject::sender()->property("InputFile").toString();

    if(pathToExample.isNull())
    {
        QString msg = "Error loading example "+pathToExample;
        emit sendErrorMessage(msg);
        return;
    }

    auto exampleName = senderObj->property("name").toString();
    emit sendStatusMessage("Loading example "+exampleName);

    auto description = senderObj->property("description").toString();

    if(!description.isEmpty())
        this->infoMessage(description);

    this->statusMessage("Loading example file.  Wait until \"Done Loading\" appears before progressing.");

    auto progressDialog = this->getProgressDialog();

    progressDialog->showProgressBar();
    progressDialog->setProgressBarRange(0,0);
    progressDialog->setProgressBarValue(0);

    QApplication::processEvents();

    this->loadFile(pathToExample);
    progressDialog->hideProgressBar();

    QString msgText("Done loading setup configuration for the example. Click the \"PREPROCESS\" button to perform preprocessing on this setup.");
    this->infoMessage(msgText);

    QString msgText2("~~~~~~~~~~\n");
    this->statusMessage(msgText2);

    QMessageBox msgBox;
    msgBox.setText(msgText);
    msgBox.exec();
}


void WorkflowAppOpenSRA::replyFinished(QNetworkReply */*pReply*/)
{
    return;
}


QGISVisualizationWidget *WorkflowAppOpenSRA::getVisualizationWidget() const
{
    return theVisualizationWidget;
}


GeneralInformationWidget *WorkflowAppOpenSRA::getGeneralInformationWidget() const
{
    return theGenInfoWidget;
}


bool WorkflowAppOpenSRA::outputToJSON(QJsonObject &jsonObjectTop)
{
    bool res = true;

    // Get each of the main widgets to output themselves
    res = theGenInfoWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    //    res = theUQWidget->outputToJSON(jsonObjectTop);

    //    if(!res)
    //        return false;

    res = thePipelineNetworkWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theIntensityMeasureWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theEDPWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theDecisionVariableWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theDamageMeasureWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theRandomVariableWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    res = theGISDataWidget->outputToJSON(jsonObjectTop);

    if(!res)
        return false;

    return true;
}


void WorkflowAppOpenSRA::postprocessResults(QString resultsDirectory, QString /*doesNothing2*/, QString /*doesNothing3*/)
{
    if(resultsDirectory.isEmpty())
        resultsDirectory = OpenSRAPreferences::getInstance()->getLocalWorkDir() + QDir::separator() + "Results";

//    auto res = theResultsWidget->processResults(resultsDirectory);
    auto res = theCustomVisualizationWidget->processResults(resultsDirectory);

    theRunWidget->hide();

    if(res == 0)
    {
        this->infoMessage("Analysis complete. Results loaded in the \"Visualization\" tab.");
        this->statusMessage("~~~~~~~~~~\n");
        theComponentSelection->displayComponent("Visualization");
//        theComponentSelection->displayComponent("Results");
    }
    else
    {
        this->infoMessage("Failed to load the results.\n");
    }

}


void WorkflowAppOpenSRA::preprocessingDone(void)
{
    auto  preprocessingDir = OpenSRAPreferences::getInstance()->getLocalWorkDir();

    QDir dirWork(preprocessingDir);
    if (!dirWork.exists())
    {
        QString errMsg = "Error, the preprocessing directory: " + preprocessingDir + " does not exist" ;
        this->errorMessage(errMsg);
        return;
    }

    preprocessor->loadPreprocessingResults(preprocessingDir);
}



void WorkflowAppOpenSRA::clear(void)
{
    theGenInfoWidget->clear();
    //    theUQWidget->clear();
    theGISDataWidget->clear();
    theRandomVariableWidget->clear();
    thePipelineNetworkWidget->clear();
    theIntensityMeasureWidget->clear();
    theDecisionVariableWidget->clear();
    theDamageMeasureWidget->clear();
    theEDPWidget->clear();
    theCustomVisualizationWidget->clear();
//    theResultsWidget->clear();
    localApp->clear();
}


void WorkflowAppOpenSRA::clearWorkDir(void)
{
    auto thePreferences = OpenSRAPreferences::getInstance(this);

    QString workDirectoryPath =  thePreferences->getLocalWorkDir();

    QDir workDirectory(workDirectoryPath);

    if(workDirectory.exists()) {
        workDirectory.removeRecursively();
    }

    // Create a new work dir
    if (!workDirectory.mkpath(workDirectoryPath)) {
        QString errorMessage = QString("Could not create Working Dir: ") + workDirectoryPath
                + QString(". Change the directory location in general information.");

        qDebug()<<errorMessage;

        return;
    }
}


void WorkflowAppOpenSRA::clearResultsDir(void)
{
    auto thePreferences = OpenSRAPreferences::getInstance(this);

    QString workDirectoryPath =  thePreferences->getLocalWorkDir();

    QString resultsPath = workDirectoryPath + QDir::separator() + "Results";

    QDir resultsDirectory(resultsPath);

    if(resultsDirectory.exists()) {
        resultsDirectory.removeRecursively();
    }

//    theResultsWidget->clear();
}


void  WorkflowAppOpenSRA::removeResultsLayer(void)
{
    if(!theVisualizationWidget->getLayerGroup("Results"))
        return;

    theVisualizationWidget->removeLayerGroup("Results");

    theCustomVisualizationWidget->clearResults();
}


bool WorkflowAppOpenSRA::inputFromJSON(QJsonObject &jsonObject)
{

    auto genJsonObj = jsonObject.value("General").toObject();
    if(theGenInfoWidget->inputFromJSON(genJsonObj) == false)
    {
        errorMessage("Error loading .json input file at " + theGenInfoWidget->objectName() + " panel");
        return false;
    }

    //    auto UQJsonObj = jsonObject.value("SamplingMethod").toObject();
    //    if(theUQWidget->inputFromJSON(UQJsonObj) == false)
    //    {
    //        errorMessage("Error loading .json input file at " + theUQWidget->objectName() + " panel");
    //        return false;
    //    }

    auto InfraJsonObj = jsonObject.value("Infrastructure").toObject();
    if(thePipelineNetworkWidget->inputFromJSON(InfraJsonObj) == false)
    {
        errorMessage("Error loading .json input file at " + thePipelineNetworkWidget->objectName() + " panel");
        return false;
    }

    auto IntensityMeasObj = jsonObject.value("IntensityMeasure").toObject();
    if(theIntensityMeasureWidget->inputFromJSON(IntensityMeasObj) == false)
    {
        errorMessage("Error loading .json input file at " + theIntensityMeasureWidget->objectName() + " panel");
        return false;
    }

    auto EDPObj = jsonObject.value("EngineeringDemandParameter").toObject();
    if(theEDPWidget->inputFromJSON(EDPObj) == false)
    {
        errorMessage("Error loading .json input file at " + theEDPWidget->objectName() + " panel");
        return false;
    }

    auto DamageMeasureObj = jsonObject.value("DamageMeasure").toObject();
    if(theDamageMeasureWidget->inputFromJSON(DamageMeasureObj) == false)
    {
        errorMessage("Error loading .json input file at " + theDamageMeasureWidget->objectName() + " panel");
        return false;
    }

    auto DecisionVarObj = jsonObject.value("DecisionVariable").toObject();
    if(theDecisionVariableWidget->inputFromJSON(DecisionVarObj) == false)
    {
        errorMessage("Error loading .json input file at " + theDecisionVariableWidget->objectName() + " panel");
        return false;
    }

    auto inputParamObj = jsonObject.value("InputParameters").toObject();
    if(theRandomVariableWidget->inputFromJSON(inputParamObj) == false)
    {
        errorMessage("Error loading .json input file at " + theRandomVariableWidget->objectName() + " panel");
        return false;
    }

    auto GISandCPTParamObj = jsonObject.value("UserSpecifiedData").toObject();
    if(theGISDataWidget->inputFromJSON(GISandCPTParamObj) == false)
    {
        errorMessage("Error loading .json input file at " + theGISDataWidget->objectName() + " panel");
        return false;
    }


    return true;
}


void WorkflowAppOpenSRA::onRunButtonClicked(QPushButton* button)
{
    //    theRunWidget->hide();
    //    theRunWidget->setMinimumWidth(this->width()*0.5);
    //    theRunWidget->showLocalApplication();

    this->removeResultsLayer();

    localApp->onRunButtonPressed(button);
}


void WorkflowAppOpenSRA::onPreprocessButtonClicked(QPushButton* button)
{
    this->removeResultsLayer();

    localApp->onPreprocessButtonPressed(button);
}


void WorkflowAppOpenSRA::onRemoteRunButtonClicked()
{

}


void WorkflowAppOpenSRA::onRemoteGetButtonClicked()
{


}


void WorkflowAppOpenSRA::onExitButtonClicked()
{

}


int WorkflowAppOpenSRA::getMaxNumParallelTasks()
{
    return 1;
}


void WorkflowAppOpenSRA::setUpForPreprocessingRun(QString &workingDir, QString &subDir)
{

    auto tmpDirectory = this->assembleInputFile(workingDir,subDir);

    if(tmpDirectory.isEmpty())
    {
        errorMessage("Error setting up the input file for preprocessing.  The preprocessing step did not run.");
        return;
    }

    QString inputDirectory = tmpDirectory + QDir::separator();

    statusMessage("Set up done.  Now starting the preprocessing.");

    emit setUpForPreprocessingDone(workingDir, inputDirectory);
}


void WorkflowAppOpenSRA::setUpForApplicationRun(QString &workingDir, QString &subDir)
{
    auto analysisdir = workingDir;

//    auto runDir = workingDir + QDir::separator() + "run";

//    QDir dir(runDir);
//    if(!dir.mkpath("."))
//    {
//        this->errorMessage("Failed to make the directory "+runDir);
//        return;
//    }


//    statusMessage("Copying files from preprocess directory to run directory.");

//    // Copy everything from the preprocess dir to the run dir
//    auto res = SCUtils::recursiveCopy(preprocessdir, runDir);

//    if(!res)
//    {
//        QString msg = "Error copying files over to the directory " + runDir;
//        this->errorMessage(msg);

//        return;
//    }

    //
    // clear Results tab of loaded any results
//    theResultsWidget->clear();
    //

    statusMessage("Set up done.  Now starting the analysis.");

//    emit setUpForApplicationRunDone(runDir, runDir);
      emit setUpForApplicationRunDone(analysisdir, analysisdir);
}


QString WorkflowAppOpenSRA::assembleInputFile(QString &workingDir, QString &subDir)
{
    //
    // create temporary directory in working dir
    // and copy all files needed to this directory by invoking copyFiles() on app widgets
    //

    QDir workDir(workingDir);

    QString tmpDirectory = workDir.absoluteFilePath(subDir);
    QDir destinationDirectory(tmpDirectory);

    if(destinationDirectory.exists()) {
        destinationDirectory.removeRecursively();
    } else
        destinationDirectory.mkpath(tmpDirectory);

    destinationDirectory.mkpath(tmpDirectory);

    // copyPath(path, tmpDirectory, false);
    auto copyOk = theIntensityMeasureWidget->copyFiles(tmpDirectory);

    if(!copyOk)
    {
        this->errorMessage("Failed to copy files in "+theIntensityMeasureWidget->objectName());
        return QString();
    }

    copyOk = theDamageMeasureWidget->copyFiles(tmpDirectory);

    if(!copyOk)
    {
        this->errorMessage("Failed to copy files in "+theDamageMeasureWidget->objectName());
        return QString();
    }

    copyOk = thePipelineNetworkWidget->copyFiles(tmpDirectory);

    if(!copyOk)
    {
        this->errorMessage("Failed to copy files in "+thePipelineNetworkWidget->objectName());
        return QString();
    }

    copyOk = theEDPWidget->copyFiles(tmpDirectory);

    if(!copyOk)
    {
        this->errorMessage("Failed to copy files in "+theEDPWidget->objectName());
        return QString();
    }


    copyOk = theRandomVariableWidget->outputToCsv(tmpDirectory);

    if(!copyOk)
    {
        this->errorMessage("Failed to output csv files in "+theRandomVariableWidget->objectName());
        return QString();
    }

    //
    // in new templatedir dir save the UI data into dakota.json file (same result as using saveAs)
    // NOTE: we append object workingDir to this which points to template dir
    //

    QString inputFile = tmpDirectory + QDir::separator() + tr("SetupConfig.json");

    QFile file(inputFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        //errorMessage();
        return QString();
    }

    QJsonObject json;
    // moved assigning of runDir here to allow access to this attribute in outputToJSON
    json["runDir"]=tmpDirectory;
    auto res = this->outputToJSON(json);
    if(!res)
    {
        errorMessage("Error setting up the analysis input file.");
        return QString();
    }

//    json["runDir"]=tmpDirectory;
    json["WorkflowType"]="OpenSRA Simulation";

    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();

    return tmpDirectory;
}


void WorkflowAppOpenSRA::loadResults(void)
{
    if(resultsDialog == nullptr)
        resultsDialog= new LoadResultsDialog(this);

    resultsDialog->show();
}


int WorkflowAppOpenSRA::loadFile(const QString fileName)
{

    //
    // Set current path to abspath of fileName for relative pathing used in setup_config.json
    QFileInfo fileInfo(fileName);
    QString absPath = fileInfo.absoluteDir().absolutePath();
    QDir::setCurrent(absPath);
    //

    //
    // open file
    //

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        this->errorMessage(QString("Could Not Open File: ") + fileName);
        return -1;
    }

    //
    // place contents of file into json object
    //

    QString val;
    val=file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject jsonObj = doc.object();

    // close file
    file.close();

    //
    // clear current and input from new JSON
    //

    this->clear();
    //this->clearWorkDir();

    // Set the current dir to the input file
    QFileInfo fileinfo( file );
    QDir::setCurrent( fileinfo.absoluteDir().path() );

    auto res = this->inputFromJSON(jsonObj);

    if(res == false)
        return -1;

    return 0;
}


void WorkflowAppOpenSRA::statusMessage(QString message)
{
    progressDialog->appendText(message);
}


void WorkflowAppOpenSRA::infoMessage(QString message)
{
    progressDialog->appendInfoMessage(message);
}


void WorkflowAppOpenSRA::errorMessage(QString message)
{
    progressDialog->appendErrorMessage(message);
}


void WorkflowAppOpenSRA::fatalMessage(QString message)
{
    progressDialog->appendErrorMessage(message);
}


RandomVariablesWidget *WorkflowAppOpenSRA::getTheRandomVariableWidget() const
{
    return theRandomVariableWidget;
}


PipelineNetworkWidget *WorkflowAppOpenSRA::getThePipelineNetworkWidget() const
{
    return thePipelineNetworkWidget;
}


WidgetFactory* WorkflowAppOpenSRA::getTheWidgetFactory() const
{
    return theWidgetFactory.get();
}


QJsonObject WorkflowAppOpenSRA::getMethodsAndParamsObj() const
{
    return methodsAndParamsObj;
}


QMap<QString, QString> WorkflowAppOpenSRA::getMethodsAndParamsMap() const
{
    return methodsParamsMap;
}
