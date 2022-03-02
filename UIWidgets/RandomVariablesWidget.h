#ifndef RANDOM_VARIABLES_CONTAINER_H
#define RANDOM_VARIABLES_CONTAINER_H

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

#include <SimCenterWidget.h>

class RandomVariable;
class RVTableView;
class ComboBoxDelegate;

class QVBoxLayout;
class QDialog;
class QCheckBox;

class RandomVariablesWidget : public SimCenterWidget
{
    Q_OBJECT
public:
    explicit RandomVariablesWidget(QWidget *parent = 0);
    ~RandomVariablesWidget();

    void addRandomVariable(RandomVariable *theRV);
    bool inputFromJSON(QJsonObject &rvObject);
    bool outputToJSON(QJsonObject &rvObject);

    //void setInitialConstantRVs(QStringList &varNamesAndValues);

    void addRandomVariable(QString &rvName);
    void addRVs(QStringList &varNames);
    void addRVsWithValues(QStringList &varNames);  
    void addConstantRVs(QStringList &varNamesAndValues);
    void addUniformRVs(QStringList &varNamesAndValues);
    void addNormalRVs(QStringList &varNamesAndValues);

    void removeRandomVariable(QString &varName);
    void removeRandomVariables(QStringList &varNames);

    QStringList getRandomVariableNames(void);
    int getNumRandomVariables(void);
    QVector<RandomVariable *> getRVdists(void);
    void copyRVs(RandomVariablesWidget *oldRVcontainers);
    void copyFiles(QString fileName);

public slots:
   void addRandomVariable(void);
   void removeRandomVariable(void);

   void loadRVsFromJson(void);
   void saveRVsToJson(void);

   void clear(void);

private:
    int addRVsType;
    void makeRVWidget(void);
    QVBoxLayout *verticalLayout;

    ComboBoxDelegate* distTypeComboDelegate = nullptr;
    ComboBoxDelegate* sourceComboDelegate = nullptr;

    RVTableView* theRVTableView = nullptr;

    QVector<RandomVariable *>theRandomVariables;
    QCheckBox *checkbox;

    QStringList randomVariableNames;
};

#endif // RANDOM_VARIABLES_CONTAINER_H