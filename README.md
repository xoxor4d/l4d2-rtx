<h1 align="center">Left 4 Dead 2 - RTX Remix Compatibility Mod</h1>

<div align="center" markdown="1"> 

This client modification is specifically made for nvidia's [rtx-remix](https://github.com/NVIDIAGameWorks/rtx-remix).  
How does a shader based game work with remix? By manually reimplementing fixed function rendering :) 

<br>

__WIP__ & __Please Note:__  
RTX Remix was never intented to support this game so expect stuff to be broken.  
__This is not trying to be a remaster__. It simply makes the game compatible with RTX Remix.

Please keep that in mind.

</div>

<br>

<div align="center" markdown="1">

![img](.github/img/01.png)
![img](.github/img/02.png)
</div>

<br>

<div align="center" markdown="1">

### __[ Remix Compatibility Features ]__   
🔹Most things are rendered using the fixed-function pipeline🔹  
🔹Remix friendly culling and the ability to manually override culling🔹  
🔹Ability to spawn and animate lights on events using a keyframe system🔹  
🔹Per map loading of remix config files to set remix variables🔹  
🔹Ability to animate remix variables on events🔹  
🔹Spawning of unique anchor meshes🔹  
🔹Per map fog settings and much more🔹  

<br>
<br>

If you want to support my work,  
consider buying me some coffee:  

[![ko-fi](https://xoxor4d.github.io/assets/img/social/kofi.png)](https://ko-fi.com/xoxor4d)
</div>

<br>
<br>

## Usage / Installation
- Download the latest [release](https://github.com/xoxor4d/l4d2-rtx/releases) and follow instructions found __there__.
- Start the game by executing `l4d2-rtx-launcher.exe` (_start as admin if game is installed under `Program Files`_)
- The window title should change to Left 4 Dead 2 - RTX - followed by the GitHub commit number if successful

<br>

#### 🟦 Info: 
- See the [Wiki](https://github.com/xoxor4d/l4d2-rtx/wiki) for in-depth guides on features that come with the compatibility mod 🍓
- Current releases ship with a [custom build of the remix-dxvk runtime](https://github.com/xoxor4d/dxvk-remix/tree/combine/l4d2) which includes necessary changes for L4D2 (`bin/.trex/d3d9.dll`)

#### 🟩 Remixing:  
- Infected will be really colorful when viewed in the toolkit. That is the colormap the infected shader is using. __DO NOT__ touch anything on infected meshes __BESIDES__ the normal map!
- Water surfaces have two layers. Make the top one translucent (and animate the normalmap with a spritesheet if you want). The lower layer can be used to color the water and make it interact with the flashlight.
Remove the albedo map and set a albedo color + opacity if you want. Reduce the roughness and increase the metallic amount to your liking.  

  Then, modify the alpha blending as follows:  

  - [ ] Use Legacy Alpha
  - [x] Blend Enabled
  - Alpha Test Type :: LessOrEqual

<br>

##  Credits
- [Nvidia - RTX Remix](https://github.com/NVIDIAGameWorks/rtx-remix)
- [People of the showcase discord](https://discord.gg/j6sh7JD3v9) - especially the nvidia engineers ✌️
- [imgui-blur-effect](https://github.com/3r4y/imgui-blur-effect)
- [l4d2-internal-base](https://github.com/gh-0x/l4d2-internal-base/tree/master)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [minhook](https://github.com/TsudaKageyu/minhook)
- [toml11](https://github.com/ToruNiina/toml11)

<br>

<div align="center" markdown="1">

![img](.github/img/03.png)
![img](.github/img/04.png)
</div>
