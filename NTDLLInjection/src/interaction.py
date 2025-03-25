settingPath = "C:\\Users\\AvaLi\\Documents\\programming\\Maldev\\Winjection-NativeAPI\\Winjection-NativeAPI\\NTDLLInjection\\src\\ntdllinjectionsettings.json"

import json
from subprocess import call

def info(msg):
    print("[i] " + str(msg))
def warn(msg):
    print("[-] " + str(msg))
def okay(msg):
    print("[+] " + str(msg))
def confirmInputIsInt(msg, failmsg="This is not an integer!"):
    while True:
        try:
            uInp = int(input(msg))
            return uInp
        except:
            print(failmsg)
            

exitLoop = False

print(" __   __  ___   __    _____  ___        ___   _______   ______  ___________  __      ______    _____  ___\n"
"|   |/  \|   | |  \  (\    \|   \      |   | /       | /  _   \(      _    )|  \    /      \  (\    \|   \  \n"
"|'   /    \:  | ||  | |.\\   \    |     ||  |(: ______)(: ( \___))__/  \\__/ ||  |  // ____  \ |.\\   \    | \n"
"|:  /'        | |:  | |: \.   \\  |     |:  | \/    |   \/ \        \\_ /    |:  | /  /    ) :)|: \.   \\  | \n"
" \//  /\'     | |.  | |.  \    \. |  ___|  /  // ___)_  //  \ _     |.  |    |.  |(: (____/ // |.  \    \. | \n"
" /   /  \\    | /\  |\|    \    \ | /  :|_/ )(:       |(:   _) \    \:  |    /\  |\\        /  |    \    \ | \n"
"|___/    \___|(__\_|_)\___|\____\)(_______/  \_______) \_______)    \__|   (__\_|_)\ _____/    \___|\____\) \n")


info("Attempting to load settings...")
try:
    with open(settingPath) as settingFile:
        settings = json.load(settingFile)
    settingFile.close()
    okay("Successfully loaded settings!")
except json.JSONDecodeError:
    warn("Settings JSON not detected. Please confirm your settings file exists (=ｘェｘ=)")
    exit()

while not exitLoop:
    uinp = confirmInputIsInt("\nWhat do you want to do..?\n0) Exit\n1) Inject loaded DLL\n2) Change loaded DLL\n\t\___")
    if uinp == 0:
        exitLoop = True
    if uinp == 1:
        processID = confirmInputIsInt("Enter your desired process ID to inject to: ")
        confirm = input("You are about to inject " + str(settings["dllpath"]) + " into the process under ID " + str(processID) + "\n\t\___Are you sure you want to continue? (Y/N)").lower()
        if confirm == "y":
            info("Injecting..!")
            call([settings["injectorPath"], str(processID)])
        else:
            info("Aborting...")
    if uinp == 2:
        newDllPath = input("What is the path of the DLL you want to load?\n\t\___")
        confirm = input("You are about to load the DLL at path " + str(newDllPath) + " into the Winjection program. Are you sure? (y/n)").lower()
        if confirm == "y":
            info("Loading...")
            settings["dllpath"] = str(newDllPath)
            settings = json.dumps(settings)
            with open(settingPath, "w") as settingFile:
                settingFile.write(settings)
            settingFile.close()
            settings = json.loads(settings)
            okay("New DLL loaded to Winjection (○｀ω´○)")
        else:
            info("Aborting...")
    
print("\nBye bye! (≚ᄌ≚)ƶƵ\n")
