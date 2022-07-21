Import("env")
import subprocess
import os
import os.path
import errno

subprocess.run("pip -q install kconfiglib windows-curses")

if os.name == 'nt':
    subprocess.run("pip -q install kconfiglib")

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
    config_file = "lib/lvgl_hal/sdl2/Kconfig"

    save_settings = env.GetProjectOption("custom_lvgl_native_drivers_kconfig_save_settings", "");
    output_header_file = env.GetProjectOption("custom_lvgl_native_drivers_kconfig_output_header", "")

    mkdir_p(os.path.dirname(save_settings))
    mkdir_p(os.path.dirname(output_header_file))


    comment = [line.strip() for line in comment_header.splitlines()]
    comment = [line for line in comment if line]
    print("Executing kconfig",config_file,save_settings, output_header_file )
    
    envlist = dict(os.environ)
    envlist["KCONFIG_CONFIG"] = save_settings
    envlist["KCONFIG_CONFIG_HEADER"] = "#" + "\n#".join(comment) + "\n"
    envlist["KCONFIG_AUTOHEADER"] = output_header_file
    envlist["KCONFIG_AUTOHEADER_HEADER"] = "// " + "\n// ".join(comment) + "\n"

    if os.name == 'nt':
        subprocess.call(["menuconfig", config_file], env=envlist, creationflags=subprocess.CREATE_NEW_CONSOLE)
    else:
        subprocess.call(["menuconfig", config_file], env=envlist)

    genconfig_command = ["genconfig", "--header-path", output_header_file, config_file];
    print(" ".join(genconfig_command))
    subprocess.call(genconfig_command, env=envlist)
 

env.AddCustomTarget(
    "lvgl-native-drivers-config",
    None,
    menuconfig_callback,
    title="lvgl-native-drivers-config",
    description="Executes lvgl native drivers config")

