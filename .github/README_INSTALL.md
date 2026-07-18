# Installation:

### Install using the installer:
1. Download <LINK_TO_MOD_ZIP>
2. Download <LINK_TO_INSTALLER>

3. Place both files in the same folder (_no need to copy them to your game folder_) and run `L4D2-Remix-CompMod-Installer.exe` 
4. Use the File Dialog to select your `left4dead2.exe` which is located in your Left 4 Dead 2 install folder

5. The installer will ask you if you want to download the required [base-remix-mod](https://github.com/xoxor4d/l4d2-rtx-base-mod).  
    - Press YES - The CompMod will not function correctly without it - so make sure to install it.
    - Use the `GIT` install method (recommended) because its easier and faster to update to new versions (and uses less bandwidth)

6. Make sure that you remove all custom launch arguments for L4D2 in Steam (if you have any set and use Steam to run the game)

<br>

### OR Install manually (no need to do this if you've used the installer):
1. Download <LINK_TO_MOD_ZIP>

2. Open the zip and extract all files contained inside the `L4D2-Remix-CompatibilityMod` folder into your Left 4 Dead 2 directory (next to the `left4dead2.exe`). Overwrite all when prompted.

3. Download [l4d2-rtx-base-mod](https://github.com/xoxor4d/l4d2-rtx-base-mod/archive/refs/heads/master.zip) 
4. Extract the _mods_ folder (inside of `l4d2-rtx-base-mod-master`) into your `rtx-remix` folder so that the folder structure looks like this:

```
.  
├─ ...
├─ 📁 steamapps
│  └─📁 common
│     └─📁 Left 4 Dead 2
│       ├─📁 bin
│       │ ├──📁 .trex
│       │ └──📜 d3d9.dll
│       │
│       ├── 📜 l4d2-rtx.dll
│       ├── 📜 left4dead2.exe
│       │
│       ├── 📁 l4d2-rtx
│       └── 📁 rtx-remix
│           └─📁 mods
│             └─📁 l4d24rtx
│               ├── 📜 _compatibility.usda
│               ├── 📜 _highquality.usda
│               ├── 📜 comp_c1m1_hotel.usda
│               └── ...
└── ...  
```

<br>

# Usage and general Info
- Run the game using the batch file `run-l4d2-rtx.bat`
- Alternatively, copy the following into the steam launch args for l4d2: `-insecure -steam -novid -disable_d3d9_hacks -limitvsconst -softparticlesdefaultoff -disallowhwmorph -no_compressed_verts +mat_phong 1`
> - Press `Alt + X` to open the Remix menu  
> - Press `F5` to open the Compatibility Mod menu

<br>

> [!Important]
> **Troubleshooting / Guides** -- Look into the **Wiki** if you are having issues:  
> https://github.com/xoxor4d/l4d2-rtx/wiki

