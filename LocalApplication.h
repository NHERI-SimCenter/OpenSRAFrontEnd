#ifndef LOCAL_APPLICATION_H
#define LOCAL_APPLICATION_H

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


// Written: fmckenna
// Modified by: Stevan Gavrilovic

// Purpose: a widget for submitting uqFEM jobs to HPC resource (specifically DesignSafe at moment)
//  - the widget aasks for additional info needed and provide a submit button to submit the jb when clicked.

#include "SimCenterWidget.h"
#include "Application.h"
#include "Utils/PythonProcessHandler.h"

#include <QProcess>

class QLabel;
class QPushButton;

class LocalApplication : public Application
{
    Q_OBJECT
public:
    explicit LocalApplication(QString workflowScriptName, QWidget *parent = nullptr);

    bool outputToJSON(QJsonObject &rvObject);

    bool setupDoneRunApplication(QString &tmpDirectory, QString &inputFile);
    bool setupDoneRunPreprocessing(QString &workingDir, QString &inputFile);

    void clear();

    void onRunButtonPressed(QPushButton* button);
    void onPreprocessButtonPressed(QPushButton* button);

public slots:

    int handlePreprocessDone(int res);
    int handleApplicationRunDone(int res);

signals:
    void processResults(QString doesNothing1, QString doesNothing2, QString doesNothing3);
    void preprocessingDone();

    void setupForRunDone(QString &, QString &);
    void setupForPreProcessingDone(QString &, QString &);

private:
    QString workflowScript;

    bool setUpForRun = false;

    QString workingDir;

    void setupTempDir(const QString& subDir);

    std::unique_ptr<PythonProcessHandler> theMainProcessHandler = nullptr;
    std::unique_ptr<PythonProcessHandler> thePreprocessHandler = nullptr;

    QPushButton* runButton = nullptr;
    QPushButton* preProcessButton = nullptr;

};

#endif // LOCAL_APPLICATION_H
