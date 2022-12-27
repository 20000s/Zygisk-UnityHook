# Zygisk-UnityHook

hook unity il2cpp function in one game  by zygisk plugin

## Build
You must modify local.properties to adapt your SDK path

You can also modify  /module/build.gradle file to match your NDK and CMAKE versions.

You should modify pacakge name in game.h

You should write your hook function in hook.c

Make sure you have a jdk11 environment.

on the command line
run
```
gradlew :module:assembleRelease
```
or click
```
build.bat
```

## Others
builded files in /out

And You can install it in Magisk24.0+

After reboot

you can find log in LogCat


# Credits
https://www.perfare.net/archives/1741  
[dobby](https://github.com/jmpews/Dobby)  
[Zygisk-ModuleTemplate](https://github.com/PShocker/Zygisk-ModuleTemplate)  
[Zygisk-Il2CppDumper](https://github.com/Perfare/Zygisk-Il2CppDumper)
