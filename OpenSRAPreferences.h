﻿#ifndef SIMCENTER_PREFERENCES_H
#define SIMCENTER_PREFERENCES_H

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

// Written: Stevan Gavrilovic

#include <QDialog>
#include <QCheckBox>

class QLineEdit;
class QVBoxLayout;

extern QString logFilePath;

class OpenSRAPreferences : public QDialog
{
    Q_OBJECT

private:
    explicit OpenSRAPreferences(QWidget *parent = nullptr);
    ~OpenSRAPreferences();
    static OpenSRAPreferences *theInstance;

public:
    static OpenSRAPreferences *getInstance(QWidget *parent = nullptr);
    QString getPython(void);
    QString getAppDir(void);
    QString getLocalWorkDir(void);
    QString getAppDataDir(void);
    QString getNDADataDir(void);

    void setLocalWorkDir(const QString &value);
    void setAppDataDir(const QString &value);
    void setNDADataDir(const QString &value);

public slots:
    void savePreferences(bool);
    void resetPreferences(bool);
    void quitPreferences(bool);
    void loadPreferences(void);
    void handleAppDataDirBrowseButtonClicked(void);
    void handleNDADataDirBrowseButtonClicked(void);

signals:

private:

    QLineEdit *appDataDirLineEdit;
    QLineEdit *NDADataDirLineEdit;

    QLineEdit *customPythonLineEdit;
    QLineEdit *customAppDirLineEdit;
    QCheckBox* customAppDirCheckBox;
    QCheckBox* customPythonCheckBox;

    QVBoxLayout *layout;
    QString localWorkDir;
};


#endif // SIMCENTER_PREFERENCES_H
