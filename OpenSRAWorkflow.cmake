# OpenSRAWorkflow.cmake
# OpenSRA-specific, SELECTIVE replacement for SimCenterCommon's simcenter_add_workflow().
#
# Why this exists (do NOT just call simcenter_add_workflow):
#   OpenSRA ships its OWN LocalApplication, MainWindowWorkflowApp and GeneralInformationWidget,
#   and does NOT do building-damage analysis. SimCenterCommon's simcenter_add_workflow compiles
#   EXECUTION/LocalApplication.cpp, WORKFLOW/MainWindowWorkflowApp.cpp and the SIM/*BuildingModel
#   sources, which (a) DUPLICATE OpenSRA's own same-named classes (shared include guards ->
#   AUTOMOC mocs the wrong header) and (b) drag in building-damage widgets that need InputSheetBM's
#   GeneralInformationWidget singleton. That is the root of ~100 of the first-pass build errors.
#
# This module adds ONLY the SimCenterCommon/Workflow sources the original QMake build used
# (OpenSRACommon.pri: the $$PATH_TO_COMMON/Workflow entries). Mirror OpenSRACommon.pri, not R2D.
#
# Usage:
#   include(path/to/OpenSRAWorkflow.cmake)
#   simcenter_add_opensra_workflow(<target>)

set(OPENSRA_WF_DIR "${CMAKE_CURRENT_LIST_DIR}/../SimCenterCommon/Workflow")

function(simcenter_add_opensra_workflow target)
  set(wf "${OPENSRA_WF_DIR}")

  # Include paths (OpenSRACommon.pri INCLUDEPATH $$PATH_TO_COMMON/Workflow*)
  target_include_directories(${target} PRIVATE
    "${wf}"
    "${wf}/WORKFLOW"
    "${wf}/EXECUTION"
    "${wf}/GRAPHICS"
    "${wf}/WORKFLOW/ModelViewItems"
    "${wf}/WORKFLOW/Utils"
  )

  # The exact 7 Workflow sources OpenSRA.pro/OpenSRACommon.pri compiled (+ their headers for AUTOMOC).
  # EXECUTION/Application is the base class OpenSRA's own LocalApplication derives from.
  target_sources(${target} PRIVATE
    "${wf}/WORKFLOW/SimCenterComponentSelection.cpp"
    "${wf}/WORKFLOW/CustomizedItemModel.cpp"
    "${wf}/EXECUTION/Application.cpp"
    "${wf}/GRAPHICS/SimCenterGraphPlot.cpp"
    "${wf}/GRAPHICS/qcustomplot.cpp"
    "${wf}/WORKFLOW/ModelViewItems/TreeItem.cpp"
    "${wf}/WORKFLOW/ModelViewItems/CheckableTreeModel.cpp"

    "${wf}/WORKFLOW/SimCenterComponentSelection.h"
    "${wf}/WORKFLOW/AnimatedStackedWidget.h"
    "${wf}/WORKFLOW/CustomizedItemModel.h"
    "${wf}/EXECUTION/Application.h"
    "${wf}/GRAPHICS/SimCenterGraphPlot.h"
    "${wf}/GRAPHICS/qcustomplot.h"
    "${wf}/WORKFLOW/ModelViewItems/TreeItem.h"
    "${wf}/WORKFLOW/ModelViewItems/CheckableTreeModel.h"
  )
endfunction()
