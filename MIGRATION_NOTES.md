# OpenSRA Qt5→Qt6 / QGIS3→QGIS4 / QMake→CMake migration notes

Living record of the migration. Records what changed, why, every divergence from the
CLAUDE.md §6 skeleton, and anything observed/needed **outside** `OpenSRAFrontEnd` (for
upstream reporting). Branch: `qt6-qgis4-migration` (off `csv-import-fix`).

---

## Status by session

| Session | Scope | State |
|---|---|---|
| 1 | Pre-flight | done (prereqs were initially missing; later satisfied by the user) |
| 2 | CMake authoring → clean **configure**; remove dead ArcGIS/plugin paths | **DONE — configure+generate exit 0** |
| 3 | Qt6 renames (§4); delete dead code | **DONE** (only non-QGIS code; full compile pending QGIS deps) |
| 4 | Link → full Release build | **DONE — clean build, `build/Release/OpenSRA.exe` produced** (see "Session 4" below) |

---

## Actual workspace layout (differs from CLAUDE.md §2)

Siblings live under `C:\dev\OpenSRA_GUI\`:
```
OpenSRA_GUI/
├── OpenSRAFrontEnd/   (this repo — target)
├── R2DTool/           (reference; FLAT layout, has CMakeLists.txt + R2D.cmake)
├── SimCenterCommon/   (shared; has QGIS/ + the *.cmake helper modules)
└── QGIS/              (SimCenter QGIS 4 fork; USER is building it)
```
So under CMake the paths are **single `..`**: `PATH_TO_COMMON=../SimCenterCommon`,
`PATH_TO_R2D=../R2DTool`. (The old QMake `OpenSRA.pro` used `../../…` for the previous
nested layout — not carried over.) **There is no `QGIS_DEPS` repo** (confirmed with the user).

---

## Files added (build system)

- `CMakeLists.txt` — top-level, mirrors `R2DTool/CMakeLists.txt`.
- `OpenSRA.cmake` — `simcenter_add_opensra(target)`, mirrors `R2D.cmake`. Adds OpenSRA's own
  sources (from `OpenSRA.pro`, minus `main.cpp`) **plus** the borrowed R2D source subset
  (from `OpenSRACommon.pri`'s `$$PATH_TO_R2D` entries). The SimCenterCommon Workflow /
  RandomVariables / Common sources that `OpenSRACommon.pri` + `Common.pri` used to pull are
  **not** listed here — they are added by the helper modules (avoids double-listing).
- `conanfile.py` — Conan **v2** (CMakeDeps + CMakeToolchain), mirrors
  `R2DTool/conanfile2.py`. Requires jansson, zlib, nlohmann_json, libcurl(non-Linux).
- `msvc_fix.h` — copied verbatim from R2D (`byte`/RPC clash fix + WIN32_LEAN_AND_MEAN/NOMINMAX),
  force-included on MSVC via `/FI`.
- `cmake/FindQCA.cmake` — copied verbatim from R2D.
- `.gitignore` — added `build/` and `CMakeUserPresets.json`.

## Files changed

- `OpenSRA.pro` — removed the dead `PATH_TO_QGIS_PLUGIN=../../QGISPlugin` + its
  `include($$PATH_TO_QGIS_PLUGIN/QGIS.pri)` (the missing Qt5 plugin). Kept as legacy reference
  only; CMake is now authoritative.
- `main.cpp:46` — `endl` → `Qt::endl`.
- `UIWidgets/FixedResidualsSamplingWidget.cpp` — removed dead `#include <QRegExpValidator>`
  (only usages were already commented out).
- `JsonWidgets/JsonLabel.cpp`, `JsonWidgets/JsonLineEdit.cpp` — removed dead
  `#include <QRegExpValidator>` (no live usage in either file).

## Files deleted (dead code)

- `UIWidgets/OpenSRAPostProcessor_old.cpp` + `.h` — legacy postprocessor, not in the build;
  held all the Qt5/QGIS3 hot-spots (QtCharts namespace, setPaperSize/get/setPageMargins old
  forms, `mainCanvas->extent()`). Deleted per CLAUDE.md §6 (do not port).
- `arcgisruntime.pri` — orphaned ESRI ArcGIS Runtime locator; never `include()`d. Deleted.

---

## Divergences from the CLAUDE.md §6 CMake skeleton (with reasons)

1. **Conan v2, not v1.** R2D's active recipe is `conanfile2.py` (v2, CMakeDeps+CMakeToolchain);
   the v1 `conanfile.py` (generators="qmake") is stale. OpenSRA uses a single v2 `conanfile.py`.
2. **No `QGIS_DEPS/include`.** `SimCenterCommon/QGIS/SimCenterQGIS.cmake` reads **no** caller
   variables and sources all QGIS dependency headers from the fork's own
   `…/QGIS/build/vcpkg_installed/x64-windows/include` (derived from its own location
   `…/SimCenterCommon/QGIS/../../QGIS`). So the skeleton's `${PATH_TO_QGIS_DEPS}/include` was
   dropped entirely.
3. **`SerialPort` dropped from Qt components.** Grep shows no `QSerialPort` use anywhere in
   OpenSRA's own / SimCenterCommon / borrowed-R2D compiled sources (only inside the QGIS fork,
   which is linked, not compiled). Per the brief's "only if used." Re-add only if Session-4
   linking against `qgis_gui` needs it.
4. **`WebEngineWidgets` kept.** `simcenter_add_workflow` adds `SimCenterCommon/Workflow/TOOLS/
   ShakerMaker.*` + `DRM_Model.*`, which use QtWebEngine — so it is transitively required.
5. **QCA + Qt6Keychain are optional at configure, required at link.** Both come from the QGIS
   fork's vcpkg build, which is not done yet. `find_package(QCA QUIET)` / `find_package(Qt6Keychain
   QUIET)` + an `OPENSRA_HAVE_QGIS_DEPS` guard let configure stay clean now; their include/link
   usage is wrapped in `if(QCA_FOUND)` / `if(Qt6Keychain_FOUND)`. **This is a temporary staging
   adaptation** — once the QGIS vcpkg tree exists, add it to `CMAKE_PREFIX_PATH` and they engage
   automatically (and are enforced at link).
6. **`QGIS_LIB_DIR` parameterized + guarded.** Empty cache default; the link block is wrapped in
   `if(QGIS_LIB_DIR)` so configure is clean before the fork is built. WIN32 links 4 modules
   (`app gui native core`) from `${QGIS_LIB_DIR}/<m>/qgis_<m>.lib`; APPLE links 6. No hard-coded
   developer paths (R2D's CMakeLists hard-codes `C:/Users/fmcke/...` — intentionally **not**
   mirrored).
7. **libcurl uses the `schannel` SSL backend on Windows** (`-o libcurl/*:with_ssl=schannel`)
   instead of the default openssl. Avoids building openssl (which needs nasm + strawberryperl)
   and sidesteps an SSL-cert failure reaching conancenter (see Environment issues). Deviation
   from R2D's default; revisit if openssl is specifically required.
8. **No explicit `qt_finalize_executable`.** Qt 6.10 auto-finalizes `qt_add_executable`; the
   explicit call double-finalized (warning). Removed.
9. **C++20** (mirrors R2D). Note: the Conan profile sets `compiler.cppstd=14`; CMakeLists
   overrides to 20 (benign configure warning). Consider setting cppstd=20 in the Conan profile.
10. **`OpenSRAUserPass.h` added conditionally** (`if(EXISTS)`) — it is gitignored/sensitive and
    absent on clean checkouts; mirrors R2D's `R2DUserPass.h` handling.
11. **Helper modules invoked:** `simcenter_add_common`, `simcenter_add_randomvariables`,
    `simcenter_add_qgis`, `simcenter_add_opensra`, `simcenter_add_workflow` (R2D's order, minus
    `simcenter_add_inputsheet` — OpenSRA's QMake never used InputSheetBM; add if a later compile
    needs it).

---

## Punch list A — Qt5 deprecated sites (final)

All live sites resolved; remaining hits were in the now-deleted `_old` file.
| Site | Action |
|---|---|
| `main.cpp:46` bare `endl` | → `Qt::endl` ✓ |
| `FixedResidualsSamplingWidget.cpp:9` `#include <QRegExpValidator>` | removed (dead) ✓ |
| `JsonLabel.cpp:41` `#include <QRegExpValidator>` | removed (dead) ✓ |
| `JsonLineEdit.cpp:40` `#include <QRegExpValidator>` | removed (dead) ✓ |
| `OpenSRAPostProcessor_old.cpp/.h` (QtCharts ns, setPaperSize, set/getPageMargins) | file deleted ✓ |

No `QStringRef`, `QtCharts::`, `QPalette::Background`, `QFileInfo f = QFile(`, or `.indexIn(`
anywhere in the live tree.

## Punch list B — QGIS-including files (for Session 5)

13 files include QGIS headers; **all are "light"** (they pass geometry types as strings to the
shared `QGISVisualizationWidget` and delegate canvas/extent/layer-removal to it). No live file
uses `QgsWkbTypes::*Geometry`, direct canvas `setExtent`/`extent()`, or `removeMapLayer` — the
only file that did was `OpenSRAPostProcessor_old.cpp` (deleted). Several already use the new
`Qgis::` enums (`Qgis::SymbolType`, `Qgis::MarkerShape`, `Qgis::RasterResamplingStage`). So
Session 5 should be mostly header/include verification once the build links; the §5 runtime
fixes live in the already-ported `SimCenterCommon/QGIS/QGISVisualizationWidget`.

Files: `OpenSRAPreProcessor.cpp`, `OpenSRAPostProcessor.cpp`, `CustomVisualizationWidget.cpp`,
`UserInputCPTWidget.cpp/.h`, `UserDefinedGroundMotionWidget.cpp/.h`, `BayAreaPipelineWidget.cpp`,
`LosAngelesPipelineWidget.cpp`, `NDABayAreaPipelineWidget.cpp`, `NDAStateWidePipelineWidget.cpp`,
`StateWidePipelineWidget.cpp` (+ trivial `main.cpp`, `WorkflowAppOpenSRA.cpp`).

---

## Configure procedure (reproducible)

```
cd OpenSRA_GUI/OpenSRAFrontEnd
conan install . --output-folder=build --build=missing -nr -s build_type=Release -o "libcurl/*:with_ssl=schannel"
cmake -S . -B build -G "Visual Studio 17 2022" \
  -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
  -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
  -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64"
```
Result: `-- Configuring done` / `-- Generating done`, exit 0, `build/OpenSRA.sln` generated.
MSVC 19.44 (VS 2022 BuildTools) + Qt 6.10.2 (all components present) detected. Once the QGIS
vcpkg tree exists, append `;C:/dev/OpenSRA_GUI/QGIS/build/vcpkg_installed/x64-windows` to
`-DCMAKE_PREFIX_PATH` (and set `-DQGIS_LIB_DIR=...` in Session 4).

---

## Changes / observations OUTSIDE OpenSRAFrontEnd (for upstream reporting)

- (Sessions 1–3 were read-only across repos.) **Session 4 required minimal edits to R2DTool**
  (borrowed source with Qt5 residue) — see the "Session 4" section below. SimCenterCommon and the
  QGIS fork were **not** edited (the workflow-composition divergence was handled OpenSRA-side).
- `SimCenterCommon/QGIS/SimCenterQGIS.cmake` hard-assumes the QGIS fork is a sibling **two
  levels up** (`SimCenterCommon/QGIS/../../QGIS`) and already **built** (references `build/…`
  and `build/vcpkg_installed/<triplet>/include`). Holds for our layout. No fork header-export
  patch needed so far.
- `R2DTool/CMakeLists.txt` hard-codes developer QGIS paths (`C:/Users/fmcke/...`,
  `/Users/fmckenna/...`). OpenSRA parameterizes via `QGIS_LIB_DIR` instead — recommend the same
  cleanup upstream in R2D.

## Environment issues for the user to resolve

- **Conan ↔ conancenter SSL failure:** `CERTIFICATE_VERIFY_FAILED` reaching
  `center2.conan.io`. Worked around for now by building from local cache (`-nr`) + schannel
  (no openssl). To restore full Conan remote access (needed if deps aren't cached), fix the CA
  bundle, e.g. set `conan config` `core.net.http:cacert_path` or env `SSL_CERT_FILE` /
  `REQUESTS_CA_BUNDLE` to the corporate/system CA. This will also matter for the QGIS vcpkg build.
- **QCA + Qt6Keychain** come from the QGIS fork's vcpkg build (not done yet). Configure is clean
  without them; LINK (Session 4) needs them on `CMAKE_PREFIX_PATH`.

---

## Session 4 (link) — COMPLETE → `build/Release/OpenSRA.exe` (clean build, 0 errors)

QGIS fork now built (`../QGIS/build/src` + `../QGIS/DEPS` + `../QGIS/build/vcpkg_installed`). Drove
the compile+link from 186 errors to a clean build. Generator: **VS 17 2022**, conan toolchain, **Release**.

**CMakeLists.txt — DEPS/QGIS wiring (this repo):**
- Appended `../QGIS/DEPS` and `../QGIS/build/vcpkg_installed/x64-windows` to `CMAKE_PREFIX_PATH`.
- QCA via `find_package(Qca-qt6 CONFIG REQUIRED)` (imported target `qca-qt6`); Qt6Keychain via its config
  (target `qt6keychain`); Qwt/QScintilla pointed at `DEPS/lib` + `DEPS/include`; `QGIS_LIB_DIR=../QGIS/build/src`.
  (Supersedes the §6/divergence-#5 "optional QCA/Qt6Keychain" staging — they are now REQUIRED and resolve from DEPS.)

**CMake composition — replaced `simcenter_add_workflow`:** OpenSRA ships its OWN `LocalApplication`,
`MainWindowWorkflowApp`, `GeneralInformationWidget`; the full `simcenter_add_workflow` also compiled
SimCenterCommon's same-named classes + SIM building models → duplicate-class/AUTOMOC conflicts. Added
**`OpenSRAFrontEnd/OpenSRAWorkflow.cmake`** (`simcenter_add_opensra_workflow`) with only the 7 Workflow
sources `OpenSRACommon.pri` used (incl. `AnimatedStackedWidget.h` for AUTOMOC). SimCenterCommon untouched.

**OpenSRA-side Qt6/QGIS4 source fixes:** `setMargin`→`setContentsMargins` (48 sites); `QString::SkipEmptyParts`
→`Qt::SkipEmptyParts`; `QgsRasterDataProvider::ResamplingMethod::Bilinear`→`Qgis::RasterResamplingMethod::Bilinear`;
`QString(int)`→`QString::number`; `<QDesktopWidget>`/`QApplication::desktop()->screenGeometry()`→
`QGuiApplication::primaryScreen()->geometry()`; `QJsonValue["x"]`→`.toObject().value("x")`; `jsonKeyword`
(private in R2D `MultiComponentR2D`) → inline the per-subclass literal; `insertWidgetIntoLayout` commented
(matches siblings); `IntensityMeasureWidget.cpp` += `#include "QGISVisualizationWidget.h"`;
**`OpenSRAPreferences.h` include-guard renamed** `SIMCENTER_PREFERENCES_H`→`OPENSRA_PREFERENCES_H` (it collided
with SimCenterCommon's `SimCenterPreferences.h` guard, blanking that class in the AUTOMOC aggregation → 24 errors).

**R2DTool — minimal Qt6 edits (borrowed source; user-approved):**
- `UIWidgets/AssetInputWidget.cpp` — `setMargin`→`setContentsMargins`; `hideCRS_Selection()`/`hideAssetFilePath()`
  used `QGridLayout` API (`columnCount`/`itemAtPosition`) on a `QVBoxLayout` → rewritten to `itemAt(index)`.
- `UIWidgets/{GISGasNetworkInputWidget,GISAboveGroundGasComponentInputWidget,GISWellsCaprocksInputWidget}.cpp` — `setMargin`→`setContentsMargins`.
- `ModelViewItems/MutuallyExclusiveListWidget.cpp` — `QString::SkipEmptyParts`→`Qt::SkipEmptyParts`.

**SimCenterCommon — runtime-crash fix (shared source; user-approved, upstream-reportable):**
- `QGIS/SimCenterMapcanvasWidget.cpp` (~line 68) — disabled the two connections that wire the embedded
  `legendTreeView`'s `selectionModel` to `QgisApp::updateNewLayerInsertionPoint` (`currentChanged`) and
  `QgisApp::legendLayerSelectionChanged` (`selectionChanged`). `legendTreeView` is a *second* `QgsLayerTreeView`
  sharing QgisApp's model; on layer removal it emits those signals for the just-deleted (dangling) node, and
  QgisApp's slots then read QgisApp's *own* layer-tree view against that node → `0xC0000005`. QgisApp's own view
  already drives these with correct timing and the embedded viz is read-only, so the connections are unneeded.
  **Fixes the ~24s startup crash** (verified: 60 s under cdb, no fatal exception — see Session 7 below). Likely
  benefits R2D too; candidate for upstream.

**zlib link:** minizip/ZipUtils (SimCenterCommon) compiles against the vcpkg (shared) zlib headers on the QGIS
include path and references `__imp_*` zlib symbols that conan's STATIC zlib doesn't provide. Linked the vcpkg
zlib import lib (`../QGIS/build/vcpkg_installed/x64-windows/lib/zlib.lib`); its `zlib1.dll` is already in-process
via QGIS. (The earlier `BEFORE`-prepend of conan's zlib include is redundant — conan marks its includes
`/external:I`, searched after the regular `/I`; harmless, can be removed in cleanup.)

**Result:** `cmake --build build --config Release` → 0 errors → `build/Release/OpenSRA.exe` (2.1 MB).
**Not yet done:** running the app (needs the runtime DLLs on PATH: Qt6 `bin`, `../QGIS/build/output/bin`,
`../QGIS/build/vcpkg_installed/x64-windows/bin`, `../QGIS/DEPS/bin`) + QGIS providers/resources — Session 6/7.

---

## Session 7 (runtime / packaging) — bundle manifest + a runtime crash

**Launch:** OpenSRA.exe LAUNCHES (main window "OpenSRA" shows). It originally **crashed ~24s later, on its own**,
with `0xC0000005` in `qgis_gui.dll` (`QgisApp::updateNewLayerInsertionPoint`). **RESOLVED** by the
`SimCenterMapcanvasWidget.cpp` connection fix above — verified by running under cdb (`sxd av; g`) for 60 s with
**no fatal exception** (app loaded the platform plugin / SQLite / GPU stack and kept running; only benign
`QObject::connect(... invalid nullptr ...)` warnings). The original-crash details below are kept for the record.
Reproduces with PATH or self-contained
bundle, with or without qt.conf → it is a runtime bug, not a packaging gap.

**Runtime DLL manifest (static closure = 110 non-system DLLs), by source:**
- **Qt (35)** — `C:\Qt\6.10.2\msvc2022_64\bin`, deployed via `windeployqt`. Incl. Qt6Core5Compat (QCA needs it).
- **QGIS + vcpkg deps (71)** — `../QGIS/build/output/bin` and `../QGIS/build/output` (root). qgis_core/gui/app/3d/analysis/native,
  gdal, geos/geos_c, proj_9, spatialite, sqlite3, zlib1, **libcrypto-3-x64 + libssl-3-x64** (qca-ossl runtime), arrow/parquet, poppler, etc.
  (`vcpkg_installed/x64-windows/bin` is redundant — output already has them.)
- **DEPS (3)** — `../QGIS/DEPS/bin`: qca-qt6, qscintilla2_qt6, qt6keychain.
- **qwt.dll (1)** — `../QGIS/DEPS/**lib**` (GOTCHA: Qwt installs its DLL to `lib/`, not `bin/`). Sole reason a 4-folder PATH gave `0xC0000135`.

**Qt plugins:** `windeployqt` under-deploys for a QGIS host (it only follows the EXE's own Qt deps). The **full**
`C:\Qt\6.10.2\msvc2022_64\plugins` tree is required — most importantly **`sqldrivers\qsqlite.dll`** (QGIS opens srs.db/gpkg via QSQLITE);
without it `qgis_gui` crashes in init. Bundle = copy the whole `plugins` tree next to the exe.

**QGIS providers + resources:** the SimCenter QGIS code calls `QgsApplication::initQgis()` but never `setPrefixPath`, so on THIS
machine QGIS uses its compiled-in prefix `../QGIS/build/output` → providers in `output/plugins`, resources in `output/data`
(`data/resources/qgis.db`, `data/svg`). For a PORTABLE bundle: copy `output/plugins` + `output/data` next to the exe and set
`QGIS_PREFIX_PATH` (or add `QgsApplication::setPrefixPath(applicationDirPath(), true)`). NOTE: `srs.db` is at `../QGIS/build/resources/srs.db`,
not under `output/data/resources` — verify QGIS finds the CRS db at runtime.

**QCA ossl plugin (runtime auth, not needed for the gpkg visual test):** `../QGIS/DEPS/lib/qca-qt6/crypto/qca-ossl.dll` (+ libcrypto/libssl, already in the DLL set).

**Bundle assembled into `OpenSRAFrontEnd/build/Release`:** windeployqt (Qt + plugins) + the 110-DLL closure + full Qt plugin tree.
A clean-PATH (self-contained) launch loads all modules — but hits the same ~24s crash below.

**RUNTIME CRASH (blocker, Session 5/6):** ~24s after launch, AV `0xC0000005`, faulting `qgis_gui.dll`. cdb stack
(nearest-export symbols; no QGIS PDBs):
`QgsLayerTreeRegistryBridge::layersWillBeRemoved → QgsLayerTreeGroup::removeChildNode → QgsLayerTreeNode::removeChildrenPrivate
→ QMetaObject::activate → QgisApp::updateNewLayerInsertionPoint → AV`. I.e. a layer-removal signal drives the embedded
`QgisApp`'s new-layer-insertion-point slot into an invalid deref. Diagnose vs R2D's QgisApp/layer-tree setup; a RelWithDebInfo
QGIS (PDBs) would give exact lines.
