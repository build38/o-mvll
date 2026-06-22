LLVM Objects
~~~~~~~~~~~~

These classes mirror LLVM's internal IR objects and are passed as parameters
to every :py:class:`~omvll.ObfuscationConfig` callback. They are read-only;
O-MVLL does not allow mutating IR through these bindings.

Module
------

.. py:class:: omvll.Module

   Mirrors ``llvm::Module``. Represents a single compilation unit (one
   source file). Passed as the first argument to every callback.

   .. py:attribute:: identifier
      :type: str

      The module identifier — typically the full file path as recorded by
      the compiler.

   .. py:attribute:: name
      :type: str

      The short name of the module (last component of the identifier).

   .. py:attribute:: source_filename
      :type: str

      The source filename recorded in the module's debug info.

   .. py:attribute:: instruction_count
      :type: int

      Total number of non-debug IR instructions across all functions in the
      module.

   .. py:attribute:: data_layout
      :type: str

      The data layout string describing the target platform's pointer sizes,
      endianness, and alignment rules
      (e.g. ``e-m:o-i64:64-i128:128-n32:64-S128``).

   .. py:method:: dump(file)

      Write the full LLVM IR of this module to *file*.

      :param file: Output file path.
      :type file: str

Function
--------

.. py:class:: omvll.Function

   Mirrors ``llvm::Function``. Represents a single function within a module.
   Passed as the second argument to every callback.

   .. py:attribute:: name
      :type: str

      The mangled name of the function as it appears in the object file,
      e.g. ``_ZN7_JNIEnv12NewStringUTFEPKc`` or ``main``.

   .. py:attribute:: demangled_name
      :type: str

      The human-readable demangled name,
      e.g. ``_JNIEnv::NewStringUTF(char const*)`` or ``main``.

   .. py:attribute:: nb_instructions
      :type: int

      The number of IR instructions in the function.

Struct
------

.. py:class:: omvll.Struct

   Mirrors ``llvm::StructType``. Passed to
   :py:meth:`~omvll.ObfuscationConfig.obfuscate_struct_access` when the
   pass encounters an access to a struct field.

   .. py:attribute:: name
      :type: str

      The name of the structure or class as recorded in the IR, e.g.:

      - ``struct.std::__ndk1::basic_string<char>``
      - ``struct._JNIEnv``
      - ``class.SecretString``

GlobalVariable
--------------

.. py:class:: omvll.GlobalVariable

   Mirrors ``llvm::GlobalVariable``. Passed to
   :py:meth:`~omvll.ObfuscationConfig.obfuscate_variable_access` when the
   pass encounters a reference to a global variable.

   .. py:attribute:: name
      :type: str

      The mangled name of the global variable.

   .. py:attribute:: demangled_name
      :type: str

      The demangled name of the global variable.
