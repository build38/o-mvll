Obfuscation
~~~~~~~~~~~

Config
------

.. py:class:: omvll.ObfuscationConfig

   Base class that must be subclassed to configure O-MVLL obfuscation passes.

   Override the callback methods below to control which passes are applied to
   each function. The configuration file must expose a top-level
   ``omvll_get_config()`` function that returns an instance of this class.
   Using :func:`functools.lru_cache` is recommended to avoid repeated
   instantiation:

   .. note::

      Most callbacks accept ``True``, ``False``, or ``None`` as convenience
      shorthands in addition to the dedicated option classes. ``None`` is
      handled explicitly because a Python method that reaches the end without
      a ``return`` statement implicitly returns ``None`` — so the following
      pattern works as intended:

      .. code-block:: python

         def break_control_flow(self, mod, func):
             if func.name == "secret_func":
                 return True
             # no return → None → pass disabled for everything else

      ``False`` and ``None`` are therefore equivalent and both disable the
      pass. The exceptions are :py:meth:`basic_block_duplicate` and
      :py:meth:`function_outline`, which require a probability-based option
      class and raise an error if a boolean is returned.

   .. code-block:: python

      @lru_cache(maxsize=1)
      def omvll_get_config() -> omvll.ObfuscationConfig:
          return MyConfig()

   .. py:method:: obfuscate_string(module, function, string)

      Callback invoked for every string literal found in *function*.

      In addition to returning a string encoding option class directly,
      the following convenience shorthands are accepted:

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``None``
           - :py:class:`~omvll.StringEncOptSkip`
         * - ``False``
           - :py:class:`~omvll.StringEncOptSkip`
         * - ``True``
           - :py:class:`~omvll.StringEncOptDefault`
         * - ``str``
           - :py:class:`~omvll.StringEncOptReplace`
         * - ``bytes``
           - :py:class:`~omvll.StringEncOptReplace`

      :param module: The LLVM module containing the function.
      :type module: :py:class:`~omvll.Module`
      :param function: The LLVM function containing the string.
      :type function: :py:class:`~omvll.Function`
      :param string: The raw bytes of the string literal.
      :type string: bytes

   .. py:method:: break_control_flow(module, function)

      Callback for the break-control-flow pass.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.BreakControlFlowOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.BreakControlFlowOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.BreakControlFlowOpt`\(``False``)

   .. py:method:: flatten_cfg(module, function)

      Callback for the control-flow flattening pass.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.ControlFlowFlatteningOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.ControlFlowFlatteningOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.ControlFlowFlatteningOpt`\(``False``)

   .. py:method:: obfuscate_struct_access(module, function, struct)

      Callback for obfuscating structure field accesses.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.StructAccessOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.StructAccessOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.StructAccessOpt`\(``False``)

      :param struct: The LLVM struct type being accessed.
      :type struct: :py:class:`~omvll.Struct`

   .. py:method:: obfuscate_variable_access(module, function, variable)

      Callback for obfuscating global variable accesses.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.VarAccessOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.VarAccessOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.VarAccessOpt`\(``False``)

      :param variable: The global variable being accessed.
      :type variable: :py:class:`~omvll.GlobalVariable`

   .. py:method:: obfuscate_constants(module, function)

      Callback for the opaque constants pass.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.OpaqueConstantsBool`\(``True``)
         * - ``False``
           - :py:class:`~omvll.OpaqueConstantsBool`\(``False``)
         * - ``None``
           - :py:class:`~omvll.OpaqueConstantsBool`\(``False``)
         * - ``list[int]``
           - :py:class:`~omvll.OpaqueConstantsSet`

   .. py:method:: obfuscate_arithmetic(module, function)

      Callback for the arithmetic obfuscation pass.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.ArithmeticOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.ArithmeticOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.ArithmeticOpt`\(``False``)

   .. py:method:: anti_hooking(module, function)

      Callback for the anti-hooking pass.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.AntiHookOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.AntiHookOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.AntiHookOpt`\(``False``)

   .. py:method:: indirect_branch(module, function)

      Callback for the indirect branch pass. Replaces ordinary branches
      with indirect jumps.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.IndirectBranchOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.IndirectBranchOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.IndirectBranchOpt`\(``False``)

   .. py:method:: indirect_call(module, function)

      Callback for the indirect call pass. Converts direct function calls
      into indirect ones by splitting the target address into two additive
      shares.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``True``
           - :py:class:`~omvll.IndirectCallOpt`\(``True``)
         * - ``False``
           - :py:class:`~omvll.IndirectCallOpt`\(``False``)
         * - ``None``
           - :py:class:`~omvll.IndirectCallOpt`\(``False``)

   .. py:method:: basic_block_duplicate(module, function)

      Callback for the basic block duplicate pass. Randomly selects basic
      blocks within *function* to be duplicated.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``None``
           - :py:class:`~omvll.BasicBlockDuplicateSkip`
         * - ``int`` (0–100)
           - :py:class:`~omvll.BasicBlockDuplicateWithProbability`\(``int``)
         * - ``bool``
           - fatal error

   .. py:method:: function_outline(module, function)

      Callback for the function outline pass. Randomly selects basic blocks
      within *function* to be outlined into new standalone functions.

      .. list-table::
         :header-rows: 1

         * - Return value
           - Interpretation
         * - ``None``
           - :py:class:`~omvll.FunctionOutlineSkip`
         * - ``int`` (0–100)
           - :py:class:`~omvll.FunctionOutlineWithProbability`\(``int``)
         * - ``bool``
           - fatal error

   .. py:method:: report_diff(pass_name, original, obfuscated)

      Optional callback to monitor IR-level changes produced by individual
      passes. Override to inspect before/after LLVM IR.

      :param pass_name: Name of the pass that made the change.
      :type pass_name: str
      :param original: The original LLVM IR of the function.
      :type original: str
      :param obfuscated: The obfuscated LLVM IR of the function.
      :type obfuscated: str

   .. py:method:: default_config(module, function, module_excludes, function_excludes, function_includes, probability)

      Built-in probability-based policy helper:

      - Skips if *module* matches any pattern in *module_excludes*.
      - Skips if *function* matches any pattern in *function_excludes*.
      - Enables unconditionally if *function* matches any pattern in
        *function_includes*.
      - Otherwise enables with the given *probability* (0–100).

      Typical use as a callback fallback:

      .. code-block:: python

         def break_control_flow(self, mod, func):
             return omvll.ObfuscationConfig.default_config(
                 self, mod, func, [], [], [], 10
             )

      :param module_excludes: Module name substrings to exclude.
      :type module_excludes: list[str]
      :param function_excludes: Function name substrings to exclude.
      :type function_excludes: list[str]
      :param function_includes: Function name substrings that force the pass on.
      :type function_includes: list[str]
      :param probability: Percentage chance (0–100) to apply the pass.
      :type probability: int
      :rtype: bool

Template
########

.. code-block:: python

   import omvll
   from functools import lru_cache

   class MyConfig(omvll.ObfuscationConfig):
       def __init__(self):
           super().__init__()

       def obfuscate_string(self, module: omvll.Module, func: omvll.Function,
                            string: bytes):
           if func.demangled_name == "Hello::say_hi()":
               return omvll.StringEncOptDefault()
           if "debug.cpp" in module.name:
               return omvll.StringEncOptReplace("<REMOVED>")
           return omvll.StringEncOptSkip()

       def obfuscate_arithmetic(self, mod: omvll.Module, func: omvll.Function):
           return omvll.ArithmeticOpt(True)

       def flatten_cfg(self, mod: omvll.Module, func: omvll.Function):
           return omvll.ControlFlowFlatteningOpt(True)

       def break_control_flow(self, mod: omvll.Module, func: omvll.Function):
           return omvll.ObfuscationConfig.default_config(
               self, mod, func, [], [], [], 10
           )

       def indirect_call(self, mod: omvll.Module, func: omvll.Function):
           return omvll.IndirectCallOpt(True)

       def function_outline(self, mod: omvll.Module, func: omvll.Function):
           return omvll.FunctionOutlineWithProbability(10)

       def basic_block_duplicate(self, mod: omvll.Module, func: omvll.Function):
           return omvll.BasicBlockDuplicateWithProbability(10)


   @lru_cache(maxsize=1)
   def omvll_get_config() -> omvll.ObfuscationConfig:
       return MyConfig()

Options
-------

Anti-Hooking
############

.. py:class:: omvll.AntiHookOpt(value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.anti_hooking` callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

Arithmetic Obfuscation
######################

.. py:class:: omvll.ArithmeticOpt(rounds_or_value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_arithmetic`
   callback. Defines the number of rounds to apply to arithmetic expressions.

   :param rounds_or_value: Number of rounds (``int``, 0–255), or a boolean
      (``True`` uses O-MVLL's default of 3 rounds, ``False`` disables the pass).
   :type rounds_or_value: int or bool

   Examples::

      ArithmeticOpt(3)     # 3 explicit rounds
      ArithmeticOpt(True)  # O-MVLL default (3 rounds)
      ArithmeticOpt(False) # disabled

Basic Block Duplicate
#####################

.. py:class:: omvll.BasicBlockDuplicateSkip

   Option for the :py:meth:`~omvll.ObfuscationConfig.basic_block_duplicate`
   callback. Disables the pass for the current function.

.. py:class:: omvll.BasicBlockDuplicateWithProbability(probability)

   Option for the :py:meth:`~omvll.ObfuscationConfig.basic_block_duplicate`
   callback. Selects basic blocks to duplicate with the given probability.

   :param probability: Percentage chance (0–100) for each basic block to be
      duplicated. ``0`` means never, ``100`` duplicates every block.
   :type probability: int

Control-Flow Breaking
#####################

.. py:class:: omvll.BreakControlFlowOpt(value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.break_control_flow`
   callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

Control-Flow Flattening
#######################

.. py:class:: omvll.ControlFlowFlatteningOpt(value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.flatten_cfg` callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

Function Outline
################

.. py:class:: omvll.FunctionOutlineSkip

   Option for the :py:meth:`~omvll.ObfuscationConfig.function_outline`
   callback. Disables the pass for the current function.

.. py:class:: omvll.FunctionOutlineWithProbability(probability)

   Option for the :py:meth:`~omvll.ObfuscationConfig.function_outline`
   callback. Selects basic blocks to outline into new functions with the
   given probability.

   :param probability: Percentage chance (0–100) for each candidate block to
      be outlined. ``0`` means never, ``100`` outlines every candidate.
   :type probability: int

Indirect Branch
###############

.. py:class:: omvll.IndirectBranchOpt(value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.indirect_branch`
   callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

Indirect Call
#############

.. py:class:: omvll.IndirectCallOpt(value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.indirect_call` callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

Opaque Constants
################

.. py:class:: omvll.OpaqueConstantsSkip

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_constants`
   callback. Disables the pass for the current function. Alias for
   :py:class:`~omvll.OpaqueConstantsBool`\(``False``).

.. py:class:: omvll.OpaqueConstantsBool(value, arith_rounds=0)

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_constants`
   callback. Obfuscates all constants (``True``) or none (``False``).

   :param value: ``True`` protects all constants, ``False`` disables the pass.
   :type value: bool
   :param arith_rounds: Additional arithmetic obfuscation rounds applied to
      the generated opaque expressions. Default ``0``.
   :type arith_rounds: int

.. py:class:: omvll.OpaqueConstantsLowerLimit(limit, arith_rounds=0)

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_constants`
   callback. Obfuscates only constants whose value is at or above *limit*.

   :param limit: Lower bound; constants below this value are left unprotected.
   :type limit: int
   :param arith_rounds: Additional arithmetic obfuscation rounds. Default ``0``.
   :type arith_rounds: int

   Examples::

      OpaqueConstantsLowerLimit(100)
      OpaqueConstantsLowerLimit(100, arith_rounds=2)

.. py:class:: omvll.OpaqueConstantsSet(constants, arith_rounds=0)

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_constants`
   callback. Obfuscates only the constants in the given list.

   :param constants: Specific constant values to protect.
   :type constants: list[int]
   :param arith_rounds: Additional arithmetic obfuscation rounds. Default ``0``.
   :type arith_rounds: int

   Examples::

      OpaqueConstantsSet([0x1234, 1, 2])
      OpaqueConstantsSet([1, 2], arith_rounds=2)

.. py:class:: omvll.OpaqueConstantsExcludeSet(constants, arith_rounds=0)

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_constants`
   callback. Obfuscates all constants **except** those in the given list.

   :param constants: Constant values to leave unprotected.
   :type constants: list[int]
   :param arith_rounds: Additional arithmetic obfuscation rounds. Default ``0``.
   :type arith_rounds: int

   Examples::

      OpaqueConstantsExcludeSet([0, 1])
      OpaqueConstantsExcludeSet([0, 1], arith_rounds=2)

Opaque Fields Access
####################

.. py:class:: omvll.StructAccessOpt(value)

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_struct_access`
   callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

.. py:class:: omvll.VarAccessOpt(value)

   Option for the
   :py:meth:`~omvll.ObfuscationConfig.obfuscate_variable_access` callback.

   :param value: ``True`` enables the protection, ``False`` disables it.
   :type value: bool

Strings Encoding
################

.. py:class:: omvll.StringEncOptSkip

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_string`
   callback. Leaves the string unprotected.

.. py:class:: omvll.StringEncOptDefault

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_string`
   callback. Defers the choice of encoding strategy to O-MVLL.

.. py:class:: omvll.StringEncOptGlobal

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_string`
   callback. Decodes the string in a global constructor (before ``main``).

   .. warning::

      The string is briefly visible in clear memory as soon as the binary
      is loaded.

.. py:class:: omvll.StringEncOptLocal

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_string`
   callback. Decodes the string lazily at the point of use within the
   function.

   .. danger::

      For large strings this can introduce significant overhead if called
      in a loop.

.. py:class:: omvll.StringEncOptReplace(new_string='')

   Option for the :py:meth:`~omvll.ObfuscationConfig.obfuscate_string`
   callback. Replaces the original string with *new_string*.

   :param new_string: The replacement string. Defaults to an empty string.
   :type new_string: str
