import setuptools
import os
import os.path
from setuptools import setup
from setuptools.extension import Extension
from distutils.file_util import copy_file
from Cython.Build import cythonize

sourcefiles = ["robotpy_sim_core.pyx", "plugin_ext_interface.c"]
extensions = [Extension(
    "robotpy_sim_core",
    sourcefiles,
    export_symbols=[
        "load_robot",
        "finalize_python",
        "robot_step",
        "set_logging_function",
        "set_robot_mode",
        "get_pwm_value",
        "set_joystick_axis",
        "set_joystick_button",
    ]
)]

class BuildExtWithNormalizedFilename(setuptools.command.build_ext.build_ext):
    def run(self):
        setuptools.command.build_ext.build_ext.run(self)

        # assumes only one extension is being built
        ext_fullname = self.get_ext_fullname(self.extensions[0].name)
        ext_filename = self.get_ext_filename(ext_fullname)
        source_filename = os.path.join(self.build_lib, ext_filename)

        build_py = self.get_finalized_command('build_py')

        output_filename = ext_fullname
        if os.name == 'nt':
            output_filename += '.dll'
        else:
            output_filename += '.so'

        output_filename = os.path.join('../Plugins', output_filename)

        copy_file(
            source_filename, output_filename, verbose=self.verbose,
            dry_run=self.dry_run
        )

setup(
    cmdclass = {
        'build_ext': BuildExtWithNormalizedFilename
    },
    ext_modules = cythonize(extensions)
)
