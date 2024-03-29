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

// Written by: Dr. Stevan Gavrilovic, UC Berkeley

#include "RVTableModel.h"
#include "RV.h"

#include <QDataStream>
#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QUuid>

RVTableModel::RVTableModel(QObject *parent) : QAbstractTableModel(parent)
{    
    numRows = 0;
    numCols = 0;
}


RVTableModel::~RVTableModel()
{

}


// Create a method to populate the model with data:
void RVTableModel::populateData(const QVector<RV>& data)
{
    tableData = data;

    this->update();

    return;
}


void RVTableModel::clear(void)
{
    numRows = 0;
    numCols = 0;

    tableData.clear();
    headerStringList.clear();
}


void RVTableModel::update()
{
    numRows = rowCount();
    numCols = columnCount();

    emit layoutChanged();
}

Qt::ItemFlags RVTableModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);

    if(!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}


int RVTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return tableData.size();
}


int RVTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if(tableData.isEmpty())
        return 0;

    return tableData.front().size();
}


QVariant RVTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    auto col = index.column();
    auto row = index.row();

    return this->item(row,col);
}


bool RVTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || value == "")
        return false;

    auto col = index.column();
    auto row = index.row();

    if(col>= numCols || row>= numRows || row < 0 || col < 0)
        return false;

    tableData[row][col] = value;
    emit handleCellChanged(row,col);

    return true;
}


QVariant RVTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(headerStringList.isEmpty())
        return QVariant();

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return headerStringList[section];

    return QVariant();
}


QVariant RVTableModel::item(const int row, const int col) const
{
    if(col>= numCols || row>= numRows || row < 0 || col < 0)
        return QVariant();

    return tableData[row][col];
}


bool RVTableModel::addRandomVariable(RV& newRV)
{
    tableData.push_back(newRV);

    this->update();

    return true;
}


int RVTableModel::getNumRVs()
{
    return tableData.size();
}


bool RVTableModel::removeRandomVariable(const RV& RVRemove, const QString& fromModel)
{
    auto rvName = RVRemove.getUuid();

    return this->removeRandomVariable(rvName,fromModel);
}


bool RVTableModel::removeRandomVariable(const QString& rvuuid, const QString& fromModel)
{
    if(tableData.isEmpty())
        return false;

    // Get the index of the RV in the table
    auto removeIdx = -1;

    for(int i = 0; i<tableData.size(); ++i)
    {
        RV& rv = tableData[i];

        if(rv.getUuid().compare(rvuuid) == 0)
        {
            removeIdx = i;

            // Remove the model from the RV list first, the RC could be shared by multiple models
            auto res = rv.removeModelFromList(fromModel);

            this->update();

            // Model removed from list, but still models in list so RV is shared, do not remove from table
            if(res == 0)
                return true;
            else if(res == -1) // Error
                return false;
            else
                break;
        }
    }

    if(removeIdx == -1)
        return false;

    tableData.remove(removeIdx);

    return true;
}


QVector<RV>& RVTableModel::getParameters()
{
    return tableData;
}


QStringList RVTableModel::getHeaderStringList() const
{
    return headerStringList;
}


void RVTableModel::setHeaderStringList(const QStringList &newHeaderStringList)
{
    headerStringList = newHeaderStringList;
    emit layoutChanged();
}


QVector<QStringList> RVTableModel::getTableData(void)
{
    QVector<QStringList> results;

    auto params = this->getParameters();

    results.reserve(params.size());

    for(auto&& param : params)
    {
        auto data = param.getDataAsStringList();

        results.push_back(data);
    }

    return results;
}


bool RVTableModel::updateRV(const QString& name, const QMap<QString, QString>& values)
{

    for(auto&& it: tableData)
    {
        if(it.getName().compare(name) == 0)
        {
            for(auto&& key : values.keys())
            {
                if(!it.updateValue(key,values.value(key)))
                    return false;
            }

            break;
        }
    }

    return true;
}
