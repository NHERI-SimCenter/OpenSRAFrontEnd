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

#include "RunWidget.h"
#include "Application.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QJsonObject>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStackedWidget>

extern int getNumParallelTasks();

RunWidget::RunWidget(Application *localA, SimCenterWidget **theWidgets, int numWidget, QWidget *parent)
    : SimCenterWidget(parent), localApp(localA), theCurrentApplication(0)
{
    theStackedWidget = new QStackedWidget();

    theCurrentApplication = localA;

    QVBoxLayout *layout = new QVBoxLayout();
    for (int i=0; i<numWidget; i++)
        layout->addWidget(theWidgets[i]);

    layout->addWidget(theStackedWidget);

    this->setLayout(layout);
}

bool RunWidget::outputToJSON(QJsonObject &jsonObject)
{
    return theCurrentApplication->outputToJSON(jsonObject);
}


bool RunWidget::inputFromJSON(QJsonObject &jsonObject)
{
    return theCurrentApplication->inputFromJSON(jsonObject);
}


void RunWidget::showLocalApplication(void)
{
    theStackedWidget->setCurrentIndex(0);
    theCurrentApplication = localApp;
    theCurrentApplication->displayed();
}



void RunWidget::setupForRunApplicationDone(QString &tmpDirectory, QString &inputFile)
{
    // qDebug() << "RunWidget::setupForRunApplicationDone";
    theCurrentApplication->setupDoneRunApplication(tmpDirectory, inputFile);
}


void RunWidget::setupForRunPreprocessingDone(QString &tmpDirectory, QString &inputFile)
{
    // qDebug() << "RunWidget::setupForRunApplicationDone";
    theCurrentApplication->setupDoneRunPreprocessing(tmpDirectory, inputFile);
}
