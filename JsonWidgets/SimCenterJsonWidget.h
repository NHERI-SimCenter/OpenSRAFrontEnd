#ifndef SimCenterJsonWidget_H
#define SimCenterJsonWidget_H
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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "SimCenterAppWidget.h"


class CustomListWidget;
class JsonDefinedWidget;
class JsonWidget;
class AddToRunListWidget;
class RandomVariablesWidget;

class QGroupBox;
class QLineEdit;

class SimCenterJsonWidget : public SimCenterAppWidget
{
public:
    SimCenterJsonWidget(QString methodName, QJsonObject jsonObj, QWidget* parent = nullptr);

    QGroupBox* getWidgetBox(const QJsonObject jsonObj);

    bool outputToJSON(QJsonObject &rvObject);
    bool inputFromJSON(QJsonObject &rvObject);

    void clear();

    // Adds a new parameter to the Input Variables widget
    bool addNewParametersToInputWidget(const QJsonObject& variablesObj, const QJsonObject& variablesTypeObj, const QString& key, QJsonObject& uuidObj);

public slots:

    void handleAddButtonPressed(void);

    void handleItemRemoved(QJsonObject removedObj);

    void handleListItemSelected(const QModelIndex &index);

private:

    QString methodKey;

    RandomVariablesWidget* theInputParamsWidget = nullptr;
    JsonDefinedWidget* methodWidget = nullptr;
    CustomListWidget* listWidget = nullptr;
    AddToRunListWidget* addRunListWidget = nullptr;

    int getVarTypes(const QJsonObject& vars, const QJsonObject& origObj, const QString& key, QJsonObject& varTypes);
};

#endif // SimCenterJsonWidget_H
