# UI Customization Guide (TL;DR)

1. Make sure your Unity `2021.3.16f1` supports both PC and Android builds. Open `UnityUI/Scenes/SampleScene.unity` and run the scene to see how the default UI works.
2. Edit `Assets/DefaultWidget/DefaultWidget.prefab` (the color data is inside `Assets/DefaultWidget/FlashAnim`).
3. Click `Assets > Build AssetBundles`. Two files are generated: `AssetBundles/defaultwidget` (PC preview only) and `AssetBundlesAndroid/defaultwidget` (Quest).
4. Copy `AssetBundlesAndroid/defaultwidget` to your Quest at `/sdcard/ModData/com.beatgames.beatsaber/Mods/HeartBeatQuest/UI/<somename>.bundle`.

# How It Works

## For Beginners — Background

The mod UI is an [AssetBundle](https://docs.unity3d.com/6000.3/Documentation/Manual/AssetBundlesIntro.html) with a [prefab](https://docs.unity3d.com/6000.3/Documentation/Manual/Prefabs.html) inside. The mod loads the prefab, mounts it to the game UI when the game starts, and destroys it when a song ends. You can add almost anything to your prefab — with some Unity limitations.

Known limitations:
- You can't add code to your UI — Unity won't load it.
- Shaders are tricky and likely won't work (not tested).
- Fonts may or may not work (probably not).

In general, create a prefab and add one or more [animators](https://docs.unity3d.com/Manual/class-AnimatorController.html). The mod will control them.

## For Pros

The Quest mod uses the prefab as implemented in [HeartController.cs](Assets/HeartController.cs). Code says everything — read it if needed.

Additional behavior:

- When loading the asset bundle, every asset is scanned. All prefabs are added to the game menu.
- All TMP_Text fonts are replaced with game fonts (otherwise they'd be invisible).
- The mod finds all Animators in the prefab. If found, parameters are set when data arrives.
- If a GameObject named `auto:heartrate` is found, its text is replaced with the heart rate when data comes in.
- All GameObjects under `info` are used to record metadata. The `name` appears in the mod menu. The `root` determines where the UI is mounted.

Available `root` values:

- `energyPanelGo`
- `songProgressPanelGO`
- `relativeScoreGo`
- `immediateRankGo`

### Animator Parameters

- `datacome` — trigger; fires when heart rate data arrives
- `heartrate` — integer; the current heart rate
- `heartpercent` — float; `heartrate / maxheart` (may exceed 1); use this for zone display
- `replaying` — bool; true if a replay is playing

- `hr_1` — integer; e.g. heart rate 123 → value 3. Add a child named `feature:digit_anim` to your `info` object to use this.
- `hr_10` — integer; e.g. heart rate 123 → value 2. Requires `feature:digit_anim` child.
- `hr_100` — integer; e.g. heart rate 123 → value 1. Requires `feature:digit_anim` child.

- `datasource` — integer; 0: Random, 1: LAN, 2: BLE, 3: OSC, 4: HypeRate, 5: Pulsoid. Add a child named `feature:datasource_anim` to your `info` object to use this.

If you want a different prefab, remember to change the AssetBundle options.

![alt text](image.png)

# About the Default Widget

Here's how the default prefab was made — so you don't miss anything.

Start with an empty Unity 3D project.

`SampleScene` is used for preview. It has a `RootCanvas` and an Image called `energyGo` that simulates the in-game UI at the bottom. `energyGo` has a `HeartController.cs` script that simulates what the mod does.

![alt text](image-1.png)

When the UI is loaded, the mod attaches it to `energyGO` as a child. Create a canvas under `energyGO` and adjust the `RectTransform` to position it correctly.

![alt text](image-2.png)

Add a TextMeshPro text to display the heart rate.

![alt text](image-3.png)

The mod searches all GameObjects for ones named `auto:heartrate` and replaces their content with the heart rate number — so name it accordingly.

![alt text](image-4.png)

You can't change the font. The mod resets all TMP fonts to the game font (otherwise all text would be invisible for reasons unknown). Use `Teko-Medium SDF` for previewing the in-game effect — it's not perfect, but good enough for sizing.

The mod finds every Animator in the prefab and controls all of them. Use your Unity skills here — this isn't a Unity tutorial.

Add more information by creating an empty GameObject called `info`. Disable it since we don't want it rendered in the scene.

![alt text](image-7.png)

Add more info elements to the UI.

![alt text](image-8.png)

The `name` is required — the mod displays it in the settings UI. Avoid duplicate names or players will be confused.

![alt text](image-9.png)

Drag the canvas into Assets to create a prefab called `Canvas`.

![alt text](image-5.png)

Change the AssetBundle option so it gets packed into the output.

![alt text](image-6.png)

You can assign multiple prefabs to the same AssetBundle — they'll be packed into a single file.

Then use `Assets > Build AssetBundles` to export. Remember to add the `.bundle` suffix to the generated file manually; the game filters out other files.

# About the `feature:xxx` Object

You don't have to do this, but it's recommended for compatibility. For example, if your UI has an info child named `feature:abc`, a mod version that doesn't support feature `abc` will show a mod update warning to the player when they try to load it.

Nothing else happens — your UI will still load on older mod versions. You can make it backward-compatible with your superb Unity animator skills if you want.
