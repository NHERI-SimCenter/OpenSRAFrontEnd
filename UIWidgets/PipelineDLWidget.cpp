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
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "PipelineDLWidget.h"
#include "sectiontitle.h"
#include "SimCenterAppSelection.h"
#include "MultiComponentDMWidget.h"
#include "MultiComponentDVWidget.h"
#include "MultiComponentEDPWidget.h"
#include "WorkflowAppR2D.h"
#include "AssetsWidget.h"

#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>


PipelineDLWidget::PipelineDLWidget(QWidget *parent) : MultiComponentR2D("OpenSRA",parent)
{
    this->setContentsMargins(0,0,0,0);

    theMainLayout->setMargin(0);
    theMainLayout->setContentsMargins(5,0,0,0);
    theMainLayout->setSpacing(0);

    theDamageMeasureWidget = new MultiComponentDMWidget();
    theEDPWidget = new MultiComponentEDPWidget();
    theDecisionVariableWidget = new MultiComponentDVWidget();

    this->addComponent("DM", theDamageMeasureWidget);
    this->addComponent("EDP", theEDPWidget);
    this->addComponent("DV", theDecisionVariableWidget);

    auto assetsWidget = WorkflowAppR2D::getInstance()->getAssetsWidget();
    auto thePipelineNetworkWidget = assetsWidget->getGasInfrastructureWidget();

    connect(thePipelineNetworkWidget,&MultiComponentR2D::selectionChangedSignal,theEDPWidget,&MultiComponentEDPWidget::handleWidgetSelected);
    connect(thePipelineNetworkWidget,&MultiComponentR2D::selectionChangedSignal,theDamageMeasureWidget,&MultiComponentDMWidget::handleWidgetSelected);
    connect(thePipelineNetworkWidget,&MultiComponentR2D::selectionChangedSignal,theDecisionVariableWidget,&MultiComponentDVWidget::handleWidgetSelected);

    this->show("DM");

}


PipelineDLWidget::~PipelineDLWidget()
{

}


bool PipelineDLWidget::outputToJSON(QJsonObject &/*jsonObject*/)
{

    return true;
}


bool PipelineDLWidget::outputAppDataToJSON(QJsonObject &jsonObject)
{

    QJsonObject jsonOut;

    if(!theDamageMeasureWidget->outputAppDataToJSON(jsonOut))
        return false;
    if(!theEDPWidget->outputAppDataToJSON(jsonOut))
        return false;
    if(!theDecisionVariableWidget->outputAppDataToJSON(jsonOut))
        return false;

    jsonObject[jsonKeyword] = jsonOut;

    return true;
}



bool PipelineDLWidget::inputFromJSON(QJsonObject &/*jsonObject*/)
{

    return true;

}


bool PipelineDLWidget::copyFiles(QString &destDir)
{

    return true;
}


void PipelineDLWidget::clear(void)
{
//    gasPipelineWidget->clear();
//    theWellsCaprocksWidget->clear();
//    theAboveGroundInfWidget->clear();
}


