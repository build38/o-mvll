Global Configuration
~~~~~~~~~~~~~~~~~~~~

.. py:attribute:: omvll.config

   The global :py:class:`~omvll.OMVLLConfig` instance. Modify its attributes
   before returning from ``omvll_get_config()`` to apply global settings:

   .. code-block:: python

      @lru_cache(maxsize=1)
      def omvll_get_config() -> omvll.ObfuscationConfig:
          omvll.config.shuffle_functions = True
          omvll.config.inline_jni_wrappers = True
          omvll.config.pass_phases = {
              omvll.Pass.Arithmetic: {omvll.Phase.Early},
          }
          return MyConfig()

.. py:class:: omvll.OMVLLConfig

   Global configuration object controlling O-MVLL's overall behavior.
   Accessed via :py:attr:`omvll.config`.

   .. py:attribute:: inline_jni_wrappers
      :type: bool

      Force inlining of JNI C++ wrapper methods such as:

      .. code-block:: cpp

         const jchar* GetStringChars(jstring string, jboolean* isCopy)
         { return functions->GetStringChars(this, string, isCopy); }

      Default: ``True``.

   .. py:attribute:: shuffle_functions
      :type: bool

      Randomize the order of functions within each module so that two builds
      of the same source do not place functions at the same relative positions.

      Default: ``True``.

   .. py:attribute:: global_mod_exclude
      :type: list[str]

      Module name substrings to exclude from all obfuscation passes. A module
      whose path contains any of these strings is skipped entirely. For
      example, adding ``"b/"`` would exclude a module at ``a/b/c/d.cpp``.

      Default: ``[]``.

   .. py:attribute:: global_func_exclude
      :type: list[str]

      Function name substrings to exclude from all obfuscation passes.

      Default: ``[]``.

   .. py:attribute:: probability_seed
      :type: int

      Seed for the random number generator used by probability-based passes
      (e.g. :py:class:`~omvll.BasicBlockDuplicateWithProbability`). A fixed
      seed makes obfuscation output deterministic across builds.

      Default: ``1``.

   .. py:attribute:: output_folder
      :type: str

      Directory where O-MVLL writes output files (e.g. log files). Created
      automatically if it does not exist. An empty string disables file output.

      Default: ``""``.

   .. py:attribute:: pass_phases
      :type: dict[Pass, set[Phase]]

      Maps each pass to the LLVM pipeline phase(s) it runs in. Passes absent
      from this dictionary default to :py:attr:`~omvll.Phase.Early`.

      .. code-block:: python

         omvll.config.pass_phases = {
             omvll.Pass.Arithmetic:       {omvll.Phase.Early},
             omvll.Pass.BreakControlFlow: {omvll.Phase.Last},
             omvll.Pass.StringEncoding:   {omvll.Phase.Early, omvll.Phase.Last},
         }

Pass Phases
-----------

.. py:class:: omvll.Phase

   Enum representing the two LLVM optimization pipeline phases where
   obfuscation passes can be registered. Used as values in
   :py:attr:`~omvll.OMVLLConfig.pass_phases`.

   .. py:attribute:: Early

      Runs before LLVM's optimizer. Default phase for all passes.

   .. py:attribute:: Last

      Runs after LLVM's optimizer.

.. py:class:: omvll.Pass

   Enum representing all available obfuscation passes. Used as keys in
   :py:attr:`~omvll.OMVLLConfig.pass_phases`.

   .. list-table::
      :header-rows: 1

      * - Value
        - Corresponding callback
      * - ``Pass.AntiHook``
        - :py:meth:`~omvll.ObfuscationConfig.anti_hooking`
      * - ``Pass.Arithmetic``
        - :py:meth:`~omvll.ObfuscationConfig.obfuscate_arithmetic`
      * - ``Pass.BasicBlockDuplicate``
        - :py:meth:`~omvll.ObfuscationConfig.basic_block_duplicate`
      * - ``Pass.BreakControlFlow``
        - :py:meth:`~omvll.ObfuscationConfig.break_control_flow`
      * - ``Pass.Cleaning``
        - (ObjC metadata cleaner — no callback)
      * - ``Pass.ControlFlowFlattening``
        - :py:meth:`~omvll.ObfuscationConfig.flatten_cfg`
      * - ``Pass.FunctionOutline``
        - :py:meth:`~omvll.ObfuscationConfig.function_outline`
      * - ``Pass.IndirectBranch``
        - :py:meth:`~omvll.ObfuscationConfig.indirect_branch`
      * - ``Pass.IndirectCall``
        - :py:meth:`~omvll.ObfuscationConfig.indirect_call`
      * - ``Pass.OpaqueConstants``
        - :py:meth:`~omvll.ObfuscationConfig.obfuscate_constants`
      * - ``Pass.OpaqueFieldAccess``
        - :py:meth:`~omvll.ObfuscationConfig.obfuscate_struct_access`,
          :py:meth:`~omvll.ObfuscationConfig.obfuscate_variable_access`
      * - ``Pass.StringEncoding``
        - :py:meth:`~omvll.ObfuscationConfig.obfuscate_string`
