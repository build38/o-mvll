Global Configuration
~~~~~~~~~~~~~~~~~~~~~~

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

.. autoclass:: omvll.OMVLLConfig
  :members: inline_jni_wrappers, shuffle_functions, global_mod_exclude, global_func_exclude, probability_seed, output_folder, pass_phases

Pass Phases
-----------

.. autoclass:: omvll.Phase

.. autoclass:: omvll.Pass
