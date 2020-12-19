# GWToolbox++

## A set of tools for Guild Wars Speed Clearers

If you are here to check toolbox features and for a download link to go [https://haskha.github.io/GWToolboxpp/](https://haskha.github.io/GWToolboxpp/). Keep reading for information on how to download and build from the source.

## How to download, build, and run
### Requirements
* CMake 3.14 or higher. Download the latest version from [https://cmake.org/download/](https://cmake.org/download/). 
* Visual Studio. You can download Visual Studio Community for free here: [https://visualstudio.microsoft.com/vs/community/](https://visualstudio.microsoft.com/vs/community/). You will also need the "Desktop development with C++" package. 
* Git. [https://git-scm.com/](https://git-scm.com/)

1. Open **Git Bash**. Use all the following commands in **Git Bash**. 

2. Clone the repository recursively: 
`git clone --recursive https://github.com/haskha/GWToolboxpp.git`

3. Navigate to the GWToolboxpp folder: 
`cd GWToolboxpp`

4. Create the build folder:
`mkdir build`

5. Move into the build folder:
`cd build`

6. Run CMake. Run **one** of the following commands, depending on which version of Visual Studio you wish to use. You can also just run `cmake .. -A Win32` and it will use the default generator. 

`cmake .. -G "Visual Studio 16 2019" -A Win32`

`cmake .. -G "Visual Studio 15 2017" -A Win32`

`cmake .. -G "Visual Studio 14 2015" -A Win32`

7. Open the project:
`cmake --open .`

8. Set the configuration to **Debug**

9. Set "GWToolbox" as the startup project. Right click on the **GWToolbox** _project_ and hit "Set as Startup Project"

10. Build the solution. 

11. Add `/localdll` (and optionally `/quiet`) to the GWToolbox project: right click on GWToolbox project -> Properties -> Debugging -> Command arguments. Keep in mind this is a per-configuration (Debug or Release) setting. This will make the launcher use the GWToolbox DLL you build, rather than the one already installed. 

10. Run. You may have to launch Visual Studio as administrator.

## Notes
* GWToolbox compiles as a DLL (`GWToolbox.dll`). `GWToolbox.exe`, aka Launcher, selects a GW clients and injects the dll, but you can also use different launchers, such as the ones in the `AutoItLauncher/` folder.
* By default, the launcher (`GWToolbox.exe`) will run the `GWToolboxdll.dll` as specified by your installation, **not** the one you compile. To run the dll in the same folder, run the launcher with command-line argument `/localdll`.
* You can use the Visual Studio debugger directly to be able to break and step through toolbox code. First launch toolbox in debug mode as normal, then go to Debug -> Attach to process, then select the gw.exe process and click Attach. You can also attach the debugger *before* running toolbox, to debug issues during launch, but then you will have to manually launch toolbox from outside visual studio, either with `GWToolbox.exe` or `AutoItLauncher/inject.au3`. 

## How to contribute
* First of all make sure you take a look at the code and respect the somewhat consistent code conventions.
* Create a new branch and commit the changes relevant to your contribution. Please make sure you don't commit unrelated lines.
* Finally submit a pull request and let us review your code. Stay updated in the pull request page for feedback and comments.

**Quick code overview:**
There are three main kinds of Toolbox components: Modules, Widgets and Windows. Any code change is most likely an additional one of those, or some change in one. 
* Modules are components without an interface, but they do have a panel in Settings. Two modules noteworthy: `GameSettings` contains any static change to the game, while `ToolboxSettings` contains any static change to toolbox.
* Widgets are visual elements without an explicit window (border, header, etc).
* Windows are visual elements with an explicit window.

Both Widgets and Windows can also have a panel in Settings, and share common code in ToolboxUIElement, which handles saving of position, visibility, etc. If you wish to create a new window/widget, please take a look at how similar ones have already been implemented.

## Credits

 **HasKha**
 * Original creator of GWToolbox++.
 
 **KAOS**
 * Original creator of the GW API used, reverse engineering work.
 * Several minor additions.

 **Ziox**   
 * Implementation of the vast majority of the chat-based features, such as custom chats and the chat timestamps.
 * Major contributor to the GW API used, reverse engineering work.
 
 **Jon**
 * Implemented many new features and improvements
 * Current maintainer of the project

 **Itecka** 
 * Original creator of a damage monitor, which inspired the toolbox damage monitor.
 * Created original implementation of the Cursor Fix.

 **Everyone who [proposed a PR](https://github.com/HasKha/GWToolboxpp/pulls?q=is%3Apr+is%3Aclosed)**

 **Misty/DarkManic**
 * Extensive work on the site and documentation.

 **and everyone working/suggesting ideas!**

**Images:**
* All images in `resources/bonds`, `resources/materials`, and `resources/pcons` are property of ArenaNet.
* All images in `resources/icons` are from www.flaticon.com 
