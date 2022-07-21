Import('env')
from os.path import join, realpath
from shutil import copyfile

project_dir =  str(env.Dir(env['BUILD_DIR']))
print("copying SDL2.dll to", project_dir)
copyfile(realpath("../i686-w64-mingw32/bin/SDL2.dll"),join(project_dir, "SDL2.dll")  );