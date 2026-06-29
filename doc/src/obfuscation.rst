Obfuscation
~~~~~~~~~~~

Config
------

.. autoclass:: omvll.ObfuscationConfig
  :members: obfuscate_string, break_control_flow, flatten_cfg, obfuscate_struct_access, obfuscate_variable_access, obfuscate_constants, obfuscate_arithmetic, anti_hooking, indirect_branch, indirect_call, basic_block_duplicate, function_outline, report_diff, default_config

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

.. autoclass:: omvll.AntiHookOpt

Arithmetic Obfuscation
######################

.. autoclass:: omvll.ArithmeticOpt

Basic Block Duplicate
#####################

.. autoclass:: omvll.BasicBlockDuplicateSkip

.. autoclass:: omvll.BasicBlockDuplicateWithProbability

Control-Flow Breaking
#####################

.. autoclass:: omvll.BreakControlFlowOpt

Control-Flow Flattening
#######################

.. autoclass:: omvll.ControlFlowFlatteningOpt

Function Outline
################

.. autoclass:: omvll.FunctionOutlineSkip

.. autoclass:: omvll.FunctionOutlineWithProbability

Indirect Branch
###############

.. autoclass:: omvll.IndirectBranchOpt

Indirect Call
#############

.. autoclass:: omvll.IndirectCallOpt

Opaque Constants
################

.. autoclass:: omvll.OpaqueConstantsSkip

.. autoclass:: omvll.OpaqueConstantsBool

.. autoclass:: omvll.OpaqueConstantsLowerLimit

.. autoclass:: omvll.OpaqueConstantsSet

.. autoclass:: omvll.OpaqueConstantsExcludeSet

Opaque Fields Access
####################

.. autoclass:: omvll.StructAccessOpt

.. autoclass:: omvll.VarAccessOpt

Strings Encoding
################

.. autoclass:: omvll.StringEncOptSkip

.. autoclass:: omvll.StringEncOptDefault

.. autoclass:: omvll.StringEncOptGlobal

.. autoclass:: omvll.StringEncOptLocal

.. autoclass:: omvll.StringEncOptReplace
