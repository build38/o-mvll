//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"

#include "omvll/utils.hpp"

#include "init.hpp"

namespace py = pybind11;

using namespace pybind11::literals;

namespace omvll {

py::module_ &py_init_llvm_bindings(py::module_ &m) {
  py::class_<llvm::Module>(m, "Module",
                           R"delim(
        Mirrors ``llvm::Module``. Represents a single compilation unit (one
        source file). Passed as the first argument to every callback.
        )delim")
      .def_property_readonly("identifier", &llvm::Module::getModuleIdentifier,
                             R"delim(
        The module identifier — typically the full file path as recorded by
        the compiler.

        :type: str
        )delim")

      .def_property_readonly(
          "name",
          [](const llvm::Module &Module) { return Module.getName().str(); },
          R"delim(
        The short name of the module (last component of the identifier).

        :type: str
        )delim")

      .def_property_readonly("source_filename",
                             &llvm::Module::getSourceFileName,
                             R"delim(
        The source filename recorded in the module's debug info.

        :type: str
        )delim")

      .def_property_readonly("instruction_count",
                             &llvm::Module::getInstructionCount,
                             R"delim(
        Total number of non-debug IR instructions across all functions in the
        module.

        :type: int
        )delim")

      .def_property_readonly("data_layout", &llvm::Module::getDataLayoutStr,
                             R"delim(
        The data layout string describing the target platform's pointer sizes,
        endianness, and alignment rules
        (e.g. ``e-m:o-i64:64-i128:128-n32:64-S128``).

        :type: str
        )delim")

      .def(
          "dump",
          [](llvm::Module &Self, const std::string &Path) { dump(Self, Path); },
          R"delim(
        Write the full LLVM IR of this module to *file*.

        :param file: Output file path.
        :type file: str
        )delim",
          "file"_a);

  py::class_<llvm::Function>(m, "Function",
                             R"delim(
        Mirrors ``llvm::Function``. Represents a single function within a
        module. Passed as the second argument to every callback.
        )delim")
      .def_property_readonly(
          "name",
          [](const llvm::Function &Func) { return Func.getName().str(); },
          R"delim(
        The mangled name of the function as it appears in the object file,
        e.g. ``_ZN7_JNIEnv12NewStringUTFEPKc`` or ``main``.

        :type: str
        )delim")
      .def_property_readonly(
          "demangled_name",
          [](const llvm::Function &Func) {
            return llvm::demangle(Func.getName().str());
          },
          R"delim(
        The human-readable demangled name,
        e.g. ``_JNIEnv::NewStringUTF(char const*)`` or ``main``.

        :type: str
        )delim")
      .def_property_readonly("nb_instructions",
                             &llvm::Function::getInstructionCount,
                             R"delim(
        The number of IR instructions in the function.

        :type: int
        )delim");

  py::class_<llvm::StructType>(m, "Struct",
                               R"delim(
        Mirrors ``llvm::StructType``. Passed to
        :py:meth:`~omvll.ObfuscationConfig.obfuscate_struct_access` when the
        pass encounters an access to a struct field.
        )delim")
      .def_property_readonly(
          "name", [](const llvm::StructType &S) { return S.getName().str(); },
          R"delim(
        The name of the structure or class as recorded in the IR, e.g.:

        - ``struct.std::__ndk1::basic_string<char>``
        - ``struct._JNIEnv``
        - ``class.SecretString``

        :type: str
        )delim");

  py::class_<llvm::GlobalVariable>(
      m, "GlobalVariable",
      R"delim(
        Mirrors ``llvm::GlobalVariable``. Passed to
        :py:meth:`~omvll.ObfuscationConfig.obfuscate_variable_access` when the
        pass encounters a reference to a global variable.
        )delim")
      .def_property_readonly(
          "name",
          [](const llvm::GlobalVariable &GV) { return GV.getName().str(); },
          R"delim(
        The mangled name of the global variable.

        :type: str
        )delim")
      .def_property_readonly(
          "demangled_name",
          [](const llvm::GlobalVariable &GV) {
            return llvm::demangle(GV.getName().str());
          },
          R"delim(
        The demangled name of the global variable.

        :type: str
        )delim");

  return m;
}

} // end namespace omvll
