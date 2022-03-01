

INCLUDEPATH +=  $$PATH_TO_R2D \
                $$PATH_TO_R2D/UIWidgets \
                $$PATH_TO_R2D/Tools \
                $$PATH_TO_R2D/Events \
                $$PATH_TO_R2D/Events/UI \
                $$PATH_TO_R2D/GraphicElements \
                $$PATH_TO_R2D/ModelViewItems \
                $$PATH_TO_COMMON/Workflow \
                $$PATH_TO_COMMON/Workflow/WORKFLOW \
                $$PATH_TO_COMMON/Workflow/EXECUTION \
                $$PATH_TO_COMMON/Workflow/GRAPHICS \
                $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems \
                $$PATH_TO_COMMON/RandomVariables \


SOURCES +=  $$PATH_TO_R2D/Tools/CSVReaderWriter.cpp \
            $$PATH_TO_R2D/Tools/XMLAdaptor.cpp \
            $$PATH_TO_R2D/Tools/AssetInputDelegate.cpp \
            $$PATH_TO_R2D/Tools/AssetFilterDelegate.cpp \
            $$PATH_TO_R2D/Tools/ComponentDatabase.cpp \
            $$PATH_TO_R2D/Tools/ComponentDatabaseManager.cpp \
            $$PATH_TO_R2D/Tools/TablePrinter.cpp \
            $$PATH_TO_R2D/Events/UI/SiteConfig.cpp \
            $$PATH_TO_R2D/Events/UI/site.cpp \
            $$PATH_TO_R2D/Events/UI/SiteGrid.cpp \
            $$PATH_TO_R2D/Events/UI/GridDivision.cpp \
            $$PATH_TO_R2D/Events/UI/Location.cpp \
            $$PATH_TO_R2D/Events/UI/SiteScatter.cpp \
            $$PATH_TO_R2D/UIWidgets/PopUpWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/LoadResultsDialog.cpp \
            $$PATH_TO_R2D/GraphicElements/GridNode.cpp \
            $$PATH_TO_R2D/GraphicElements/NodeHandle.cpp \
            $$PATH_TO_R2D/GraphicElements/RectangleGrid.cpp \
            $$PATH_TO_R2D/UIWidgets/SimCenterMapcanvasWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/GISSelectable.cpp \
            $$PATH_TO_R2D/UIWidgets/VisualizationWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/QGISGasPipelineInputWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/ComponentInputWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/ShakeMapWidget.cpp \
            $$PATH_TO_R2D/UIWidgets/GroundMotionStation.cpp \
            $$PATH_TO_R2D/UIWidgets/GroundMotionTimeHistory.cpp \
            $$PATH_TO_R2D/ModelViewItems/CustomListWidget.cpp \
            $$PATH_TO_R2D/ModelViewItems/MutuallyExclusiveListWidget.cpp \
            $$PATH_TO_R2D/ModelViewItems/ComponentTableModel.cpp \
            $$PATH_TO_R2D/ModelViewItems/ComponentTableView.cpp \
            $$PATH_TO_R2D/ModelViewItems/ListTreeModel.cpp \
            $$PATH_TO_R2D/ModelViewItems/GISLegendView.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.cpp \
            $$PATH_TO_COMMON/Workflow/EXECUTION/Application.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/SimCenterGraphPlot.cpp \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/qcustomplot.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/TreeItem.cpp \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/CheckableTreeModel.cpp \
            $$PATH_TO_COMMON/RandomVariables/RandomVariableDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/RandomVariable.cpp \
            $$PATH_TO_COMMON/RandomVariables/BetaDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/ChiSquaredDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/ContinuousDesignDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/GammaDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/GumbelDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/NormalDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/UniformDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/WeibullDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/ConstantDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/DiscreteDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/LognormalDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/ExponentialDistribution.cpp \
            $$PATH_TO_COMMON/RandomVariables/TruncatedExponentialDistribution.cpp \

HEADERS +=  $$PATH_TO_R2D/Tools/XMLAdaptor.h \
            $$PATH_TO_R2D/Tools/CSVReaderWriter.h \
            $$PATH_TO_R2D/Tools/AssetInputDelegate.h \
            $$PATH_TO_R2D/Tools/AssetFilterDelegate.h \
            $$PATH_TO_R2D/Tools/ComponentDatabase.h \
            $$PATH_TO_R2D/Tools/TablePrinter.h \
            $$PATH_TO_R2D/Tools/ComponentDatabaseManager.h \
            $$PATH_TO_R2D/Events/UI/SiteConfig.h \
            $$PATH_TO_R2D/Events/UI/site.h \
            $$PATH_TO_R2D/Events/UI/SiteGrid.h \
            $$PATH_TO_R2D/Events/UI/GridDivision.h \
            $$PATH_TO_R2D/Events/UI/Location.h \
            $$PATH_TO_R2D/Events/UI/SiteScatter.h \
            $$PATH_TO_R2D/Events/UI/JsonSerializable.h \
            $$PATH_TO_R2D/UIWidgets/SimCenterMapcanvasWidget.h \
            $$PATH_TO_R2D/UIWidgets/GISSelectable.h \
            $$PATH_TO_R2D/UIWidgets/PopUpWidget.h \
            $$PATH_TO_R2D/UIWidgets/LoadResultsDialog.h \
            $$PATH_TO_R2D/GraphicElements/GridNode.h \
            $$PATH_TO_R2D/GraphicElements/NodeHandle.h \
            $$PATH_TO_R2D/GraphicElements/RectangleGrid.h \
            $$PATH_TO_R2D/UIWidgets/VisualizationWidget.h \
            $$PATH_TO_R2D/UIWidgets/QGISGasPipelineInputWidget.h \
            $$PATH_TO_R2D/UIWidgets/ComponentInputWidget.h \
            $$PATH_TO_R2D/UIWidgets/ShakeMapWidget.h \
            $$PATH_TO_R2D/UIWidgets/GroundMotionStation.h \
            $$PATH_TO_R2D/UIWidgets/GroundMotionTimeHistory.h \
            $$PATH_TO_R2D/ModelViewItems/CustomListWidget.h \
            $$PATH_TO_R2D/ModelViewItems/MutuallyExclusiveListWidget.h \
            $$PATH_TO_R2D/ModelViewItems/ComponentTableModel.h \
            $$PATH_TO_R2D/ModelViewItems/ComponentTableView.h \
            $$PATH_TO_R2D/ModelViewItems/ListTreeModel.h \
            $$PATH_TO_R2D/ModelViewItems/GISLegendView.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/SimCenterComponentSelection.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/CustomizedItemModel.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/TreeItem.h \
            $$PATH_TO_COMMON/Workflow/WORKFLOW/ModelViewItems/CheckableTreeModel.h \
            $$PATH_TO_COMMON/Workflow/EXECUTION/Application.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/SimCenterGraphPlot.h \
            $$PATH_TO_COMMON/Workflow/GRAPHICS/qcustomplot.h \
            $$PATH_TO_COMMON/RandomVariables/RandomVariableDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/RandomVariable.h \
            $$PATH_TO_COMMON/RandomVariables/BetaDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/ChiSquaredDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/ContinuousDesignDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/GammaDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/GumbelDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/NormalDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/UniformDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/WeibullDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/ConstantDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/DiscreteDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/LognormalDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/ExponentialDistribution.h \
            $$PATH_TO_COMMON/RandomVariables/TruncatedExponentialDistribution.h \


