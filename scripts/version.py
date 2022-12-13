Import("env")
import subprocess
PIPE = subprocess.PIPE

device_firmware_version = ""
device_model_code = "RAK4631"
device_git_hash = ""

def run(*args):
    p = subprocess.Popen(['git'] + list(args), stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate()
    return stdout.decode("utf-8")

def get_latest_git_tag():
    global device_firmware_version
    device_firmware_version = run("describe", "--tags", "--abbrev=0").rstrip()

def get_latest_git_hash():
    global device_git_hash
    device_git_hash = run("rev-parse", "HEAD").rstrip()

def before_build():
    print("RUNNER VERSION SCRIPT")
    get_latest_git_tag()
    get_latest_git_hash()
    with open("{}/version.h".format(env['PROJECT_INCLUDE_DIR']), "w") as f :
        f.write('#include <Arduino.h>\r')
        f.write('#include "main.h"\r\n')
        f.write('const char device_firmware_version[6] = "' + device_firmware_version + '";\n')
        f.write('const char device_model_code[8] = "' + device_model_code + '";\n')
        f.write('const char device_git_hash[41] = "' + device_git_hash + '";\n')
        f.close()

before_build()
