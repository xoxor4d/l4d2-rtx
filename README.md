<h1 align="center">Left 4 Dead 2 - RTX Remix Compatibility Mod</h1>

<div align="center" markdown="1"> 

This client modification is specifically made for nvidia's [rtx-remix](https://github.com/NVIDIAGameWorks/rtx-remix).  
How does a shader based game work with remix? By manually reimplementing fixed function rendering :) 

<br>

<img src=".github/img/logo.png" alt="Description" width="50%">

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
consider buying me a [Coffee](https://ko-fi.com/xoxor4d) or by becoming a [Patreon](https://patreon.com/xoxor4d)

Feel free to join the discord server: https://discord.gg/FMnfhpfZy9

</div>

<br>
<br>

## Installing
- Grab the latest [Release](https://github.com/xoxor4d/l4d2-rtx/releases) and follow the instructions found there


<br>

#### 🟨 Expectancy:
Let me get this straight again: __This is not a remaster__.  
The compatibility mod itself does not enhance any assets and only handles a few lights and as such, might even look worse than the original game. This will however change when community made mods with proper enhanced assets and proper lighting get available.   
There is a [base-remix-mod](https://github.com/xoxor4d/l4d2-rtx-base-mod) that I have created that remixes things on a few maps (eg. water, a few lights) but it's really limited.

You'll encounter some of the following:
- Ugly looking water surfaces
- Rubbery looking textures
- Missing sunlight
- Very dark areas
- Missing (black) skybox (you need to assign the __Sky__ category to skybox textures via remix in-game settings)

<br>

#### 🟥 Issues:
- The renderer is forced to run single threaded (currently required)
- Alpha tested foliage kills performance. This might improve with future updates to rtx-remix itself or when community mods with _proper_ assets are available
- Skinning of meshes (animated meshes) is done via software (on the CPU) resulting in CPU bottlenecks
- Some effects look incorrect because of the limitations of fixed-function, other issues or they are simply not yet handled correctly.

<br>

#### 🟦 Info: 
- See the [Wiki](https://github.com/xoxor4d/l4d2-rtx/wiki) for in-depth guides on features that come with the compatibility mod 🍓
- Current releases ship with a [custom build of the remix-dxvk runtime](https://github.com/xoxor4d/dxvk-remix/tree/game/l4d2_rebase1) which includes necessary changes for L4D2 (`bin/.trex/d3d9.dll`)

#### 🟩 Remixing:  
- Press __F5__ to open the in-game gui to tweak compatibility mod related settings or to edit [MapSettings](https://github.com/xoxor4d/l4d2-rtx/wiki/Map-Settings)
- A few things when it comes to remixing of certain objects [Wiki](https://github.com/xoxor4d/l4d2-rtx/wiki/Remixing-Notes)

<br>

##  Credits
- [Nvidia - RTX Remix](https://github.com/NVIDIAGameWorks/rtx-remix)
- [People of the showcase discord](https://discord.gg/j6sh7JD3v9) - especially the nvidia engineers ✌️
- [Dear ImGui](https://github.com/ocornut/imgui)
- [imgui-blur-effect](https://github.com/3r4y/imgui-blur-effect)
- [minhook](https://github.com/TsudaKageyu/minhook)
- [toml11](https://github.com/ToruNiina/toml11)
- [dxwrapper](https://github.com/elishacloud/dxwrapper)
- [Miniz](https://github.com/richgel999/miniz)
- [l4d2-internal-base](https://github.com/xastrix-csgo-modules/eblenix_csgo_public/tree/54a04b5f3873e35a68d7f99d2656c54251fb098d/Left%204%20Dead%202/l4d2)
- [Entity](https://www.youtube.com/@paprykszadolowski8796)
- [KapibosRU](https://www.youtube.com/channel/UCqZ2NI_fQKRN-Onypt9aIGQ)

<br>

<div align="center" markdown="1">

![img](.github/img/03.png)
![img](.github/img/04.png)
</div>
