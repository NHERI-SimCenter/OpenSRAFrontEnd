#ifndef OPENSRAUSERPASS_H
#define OPENSRAUSERPASS_H

// Written by: Stevan Gavrilovic

// DO NOT CHANGE THE NAME OF THIS FILE, OTHERWISE GIT IGNORE WILL NOT WORK AND IT WILL INCLUDE IT IN THE REPO

#include <QString>

// Key intentionally blank: the OpenSRA build is QGIS-based and never calls
// getArcGISKey(); an empty key is sufficient. (Historical ArcGIS key removed.)
static QString getArcGISKey(void){return "";}

#endif // OPENSRAUSERPASS_H
