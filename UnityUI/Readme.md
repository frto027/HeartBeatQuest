# UI Customize Guide (TL;DR)

1. Make sure your `Unity 2021.3.16f1` support both of PC build and Android build, then open the `UnityUI/Scenes/SampleScene.unity`. Run the scene, you will see how default UI works.
2. Edit the `Assets/DefaultWidget/DefaultWidget.prefab`. (btw. the color information is inside the `Assets/DefaultWidget/FlashAnim`)
3. click `Assets > Build AssetBundles`, two files were generated. `AssetBundles/defaultwidget` is only for PC to preview, and `AssetBundlesAndroid/defaultwidget` is for quest use.
4. copy the `AssetBundlesAndroid/defaultwidget` and rename it to your quest folder `/sdcard/ModData/com.beatgames.beatsaber/Mods/HeartBeatQuest/UI/<somename>.bundle`

# How it works

## For beginners background

The mod ui is designed to be an [AssetBundle](https://docs.unity3d.com/6000.3/Documentation/Manual/AssetBundlesIntro.html) with some [prefab](https://docs.unity3d.com/6000.3/Documentation/Manual/Prefabs.html) inside it. The mod load the prefab and mount it to the game UI when game start, and destroy it when song play over. You can add nearly everything to your prefab if you want create a mod UI. But with some Unity limit.

Some limit I know:
- You can't add code to your UI, unity won't load it.
- Shader is magic and likely not work. I'm not tested.
- Font works or not? idk, maybe not.

Generally, you should create a prefab and add one or more [animator](https://docs.unity3d.com/Manual/class-AnimatorController.html), then mod will control it/them.

## For pro

The quest mod uses the prefab just as [HeartController.cs](Assets/HeartController.cs). Code says everything, read it if you need.

And with the following additional behavior:

- When loading asset bundle, every asset inside it will be scanned. And all prefab will be added to game menu.
- All TMP_Text fonts were replaced to the game fonts. If we don't do this, the font will invisible.
- The mod will find all Animator inside the prefab. If found, the parameters will be set when data come.
- if a gameobject called `auto:heartrate` is found, their texts will be replaced to heart rate when data come.
- All gameobject name below `info` is used to record your information. The `name` will display in the mod menu. The `root` will indicate where your ui is been mounted.

Avaliable `root` value:

-`energyPanelGo`  
-`songProgressPanelGO`  
-`relativeScoreGo`  
-`immediateRankGo`  

Animate Parameters:

- `datacome`, trigger, triggered when heart rate date come  
- `heartrate`, integer, the heartrate
- `heartpercent`, float, the value of `heartrate/maxheart`, it maybe larger than 1 if possible, you can use this to display heart zone informations.  
- `replaying`, bool, if we are replaying from a replay file  

- `hr_1`, integer, e.g. when heart is 123, this value is 3. Add a child called `feature:digit_anim` to your `info` object if you use this.  
- `hr_10`, integer, e.g. when heart is 123, this value is 2. Add a child called `feature:digit_anim` to your `info` object if you use this.  
- `hr_100`, integer, e.g. when heart is 123, this value is 1. Add a child called `feature:digit_anim` to your `info` object if you use this.  

- `datasource`, integer, `0:random, 1:LAN, 2:BLE, 3:OSC, 4:HypeRate, 5:Pulsoid`. You can add child called `feature:datasource_anim` to your `info` object if you use this.  

If you want another prefab, remember to change the AssetBundle options below.

![alt text](image.png)

# About the Default Widget

I will describe how did I make the default prefab here, to make sure you missed nothing.

You can start with a empty unity 3d project.

SampleScene is used to preview, it has a RootCanvas, and a Image called `energyGo` to simulate the game UI at the bottom. And energyGo added a `HeartController.cs` to simulate what the mod does.
![alt text](image-1.png)

 When UI loaded, our ui will be attached to the energyGO as the parent by the mod, so just create a canvas under the energyGO and change the `RectTransform` to make sure it is in the right place.

![alt text](image-2.png)

We can create a TextMeshPro text here to display the heartrate.

![alt text](image-3.png)

The mod will search all `GameObject`, if they called `auto:heartrate`, it's content will be replaced to the heart rate number. So rename it.

![alt text](image-4.png)

You can't change the Font, because the mod will reset the TMPs font to the game used font. If the mod not reset the font, all texts will be invisible for some reason idk. But you can use the font called `Teko-Medium SDF` to preview the effect in game, it's not a perfect preview, but can be used to design the text size.

The mod will find every Animator in the prefab and control all of them. Do this with your Unity skills, we won't go into detail since this isn't a Unity tutorial.


We could add more informathion to the scene. Create a empty GameObject called info. Disable it because we don't want it being rendered in the scene.

![alt text](image-7.png)

You can add more information to the UI.

![alt text](image-8.png)

The name is required, because the mod needs to display a name in the setthings UI. Please prevent duplicate name with other UI, or the player will be confused.

![alt text](image-9.png)
Drag and drop the canvas to the assets, we got a prefab called Canvas.

![alt text](image-5.png)

Change the AssetBundle option to make sure it will be packed to the output.

![alt text](image-6.png)

You can set multiple prefabs to a same AssetBundle value, and they will be packed to a single file.

Then use `Assets > Build AssetBundles` to export the asset bundle. Remember to add a suffix `.bundle` for the generated file manually, the game will filter other files.

# UI Options Feature

You can add some toggle to your UI with `toggle:xxx` under the info. The value `xxx` will be show at the mod setthings menu. And the animation boolean parameter `xxx` will be checked if player selected the toggle.

# About the `feature:xxx` object

You don't have to do this but you are suggested to. This is for compatibility. For example, if your UI has a child info called `feature:abc`, the mod version that doesn't support the feature `abc` will shows a mod update warning to player when they are trying to load this UI.

Nothing else will happen. Your UI will still be loaded in the old version mod. You can make it compatable to the old mod if you want, with your superb unity animator skill.
