from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os

# Conan v2 recipe for OpenSRA, mirroring R2DTool/conanfile2.py.
# Provides jansson, zlib, nlohmann_json and (non-Linux) libcurl via CMakeDeps so the
# top-level CMakeLists.txt find_package() calls resolve. QCA and the QGIS libraries are
# NOT Conan deps here: QCA is located by cmake/FindQCA.cmake, and the qgis_* libraries are
# linked directly from the user-supplied QGIS_LIB_DIR (see CMakeLists.txt).

class OpenSRA_Conan(ConanFile):
    name = "OpenSRA"
    version = "1.0.0"
    license = "BSD-3-Clause"
    author = "NHERI SimCenter"
    url = "https://github.com/NHERI-SimCenter/OpenSRA"
    description = "OpenSRA - Open-source Seismic Risk Analysis frontend"
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        self.requires("jansson/2.13.1")
        self.requires("zlib/1.3.1")
        self.requires("nlohmann_json/3.12.0")
        if self.settings.os != "Linux":
            self.requires("libcurl/8.12.1")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

        if self.settings.os == "Windows":
            # Stage dependency DLLs next to the executable (build/<build_type>)
            bindir = os.path.join(self.build_folder, str(self.settings.build_type))
            for dep in self.dependencies.values():
                if dep.cpp_info.bindirs:
                    source_dir = dep.cpp_info.bindirs[0]
                    if os.path.exists(source_dir):
                        copy(self, "*.dll", source_dir, bindir)
