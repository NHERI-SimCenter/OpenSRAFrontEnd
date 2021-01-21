#ifndef CustomVisualizationWidget_H
#define CustomVisualizationWidget_H
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

// Written by: Stevan Gavrilovic
// Latest revision: 10.01.2020

#include <SimCenterAppWidget.h>

#include <QNetworkAccessManager>

namespace Esri
{
namespace ArcGISRuntime
{
class ArcGISMapImageLayer;
class GroupLayer;
class FeatureCollectionLayer;
class KmlLayer;
}
}

class QGroupBox;
class QPushButton;
class VisualizationWidget;
class ShakeMapWidget;
class QNetworkReply;

class CustomVisualizationWidget : public SimCenterAppWidget
{

    Q_OBJECT

public:

    explicit CustomVisualizationWidget(QWidget *parent, VisualizationWidget* visWidget);
    virtual ~CustomVisualizationWidget();

    virtual bool outputToJSON(QJsonObject &rvObject);
    virtual bool inputFromJSON(QJsonObject &rvObject);

    virtual int processResults(QString &filenameResults);

signals:


public slots:
    void processNetworkReply(QNetworkReply* pReply);

    void showCGSGeologicMap(bool state);

    void showCGSLandslideMap(bool state);

    void showCGSLiquefactionMap(bool state);

    void showShakeMapLayer(bool state);

    void setCurrentlyViewable(bool status);

protected:

private:

    void createGSGLayers();

    QGroupBox* getVisSelectionGroupBox(void);
    QPushButton* loadShakeMapButton;
    std::unique_ptr<ShakeMapWidget> theShakeMapWidget;

    VisualizationWidget* theVisualizationWidget;

    QNetworkAccessManager m_WebCtrl;
    QNetworkReply* downloadJsonReply;
    QString baseCGSURL;

    Esri::ArcGISRuntime::ArcGISMapImageLayer* landslideLayer;
    Esri::ArcGISRuntime::ArcGISMapImageLayer* liquefactionLayer;
    Esri::ArcGISRuntime::ArcGISMapImageLayer* geologicMapLayer;


};

#endif // CustomVisualizationWidget
