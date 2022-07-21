Import("env")
import subprocess
import os
import os.path
import errno


if os.name == 'nt':
    subprocess.run("pip3 -q install kconfiglib windows-curses")
else:
    subprocess.run("pip3 -q install kconfiglib",shell=True)

# Taken from https://stackoverflow.com/a/600612/119527
def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def menuconfig_callback(*arg, **kwargs):
    # save_settings = "include/lvgl.config";
    # output_header_file = "include/lvgl.h"
    comment_header = "Configured by Dror Gluska"
    config_file = "lib/lvgl-8.3.0/Kconfig"

    include_header = """
#ifndef LV_CONF_H
#define LV_CONF_H
#endif

"""

    save_settings = env.GetProjectOption("custom_lvgl_kconfig_save_settings", "");
    output_header_file = env.GetProjectOption("custom_lvgl_kconfig_output_header", "")
    
    include_headers_string = env.GetProjectOption("custom_lvgl_kconfig_include_headers", "")
    include_headers = [line.strip() for line in include_headers_string.splitlines()]
    include_headers = [line for line in include_headers if line]
    include_headers = ["#include \"" + line + "\"" for line in include_headers]

    mkdir_p(os.path.dirname(save_settings))
    mkdir_p(os.path.dirname(output_header_file))


    comment = [line.strip() for line in comment_header.splitlines()]
    comment = [line for line in comment if line]
    print("Executing kconfig",config_file,save_settings, output_header_file )
    
    envlist = dict(os.environ)
    envlist["KCONFIG_CONFIG"] = save_settings
    envlist["KCONFIG_CONFIG_HEADER"] = "#" + "\n#".join(comment) + "\n"
    envlist["KCONFIG_AUTOHEADER"] = output_header_file
    envlist["KCONFIG_AUTOHEADER_HEADER"] = "// " + "\n// ".join(comment) + "\n"  + include_header + "\n".join(include_headers) + "\n\n"

    if os.name == 'nt':
        subprocess.call(["menuconfig", config_file], env=envlist, creationflags=subprocess.CREATE_NEW_CONSOLE)
    else:
        subprocess.call(["menuconfig", config_file], env=envlist)

    genconfig_command = ["genconfig", "--header-path", output_header_file, config_file];
    print(" ".join(genconfig_command))
    subprocess.call(genconfig_command, env=envlist)
 

env.AddCustomTarget(
    "lvgl-config",
    None,
    menuconfig_callback,
    title="lvgl-config",
    description="Executes lvgl config")

