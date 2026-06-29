LLVM Objects
~~~~~~~~~~~~

These classes mirror LLVM's internal IR objects and are passed as parameters
to every :py:class:`~omvll.ObfuscationConfig` callback. They are read-only;
O-MVLL does not allow mutating IR through these bindings.

Module
------

.. autoclass:: omvll.Module
  :members: identifier, name, source_filename, instruction_count, data_layout, dump

Function
--------

.. autoclass:: omvll.Function
  :members: name, demangled_name, nb_instructions

Struct
------

.. autoclass:: omvll.Struct
  :members: name

GlobalVariable
--------------

.. autoclass:: omvll.GlobalVariable
  :members: name, demangled_name
