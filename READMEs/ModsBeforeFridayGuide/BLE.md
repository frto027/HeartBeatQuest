# Bluetooth Permission Guide for MBF

If you want to connect your Bluetooth heart rate device directly to the game, follow this guide.

There are two ways for MBF to patch Bluetooth permission. The first is to set up the permission before patching the game. The second is to re-patch the permission after the game is already patched.

> [!NOTE]
> **PAIRING vs. BLUETOOTH PERMISSION**  
> If you only turn on Bluetooth permission, the mod will probably work — but only with **paired** Bluetooth devices.  
> If you follow this guide, the mod will be able to connect your HR device without pairing. This is the recommended setup.  
> **Please unpair your heart rate device from your Quest's OS if you follow this guide. No pairing is needed.**

If you're installing a fresh game, follow the next section. If your game is already modded, skip to [Repatching an Already Modded Game](#repatching-an-already-moded-game).

## Setup with a New Game Install

Before patching the game with MBF, enable the Bluetooth permission toggle. MBF will then patch Bluetooth for you.

![alt text](image.png)

Turn on Bluetooth permission, then click the `Advanced Options` button.

![alt text](image-2.png)

Continue to the next section.

### Edit XML Guide

Click `Edit XML`.

![alt text](image-3.png)

Click `Download Current XML`.

![alt text](image-4.png)

Right-click the downloaded file and click `Edit`.

Copy the following text into the XML file (see screenshot):

```xml
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="65536" />
```

![alt text](image-5.png)

Save the XML file, then click `Upload XML` and select the file you just edited.

![alt text](image-6.png)

Then click `Confirm Permission`.

![alt text](image-7.png)

Finally, click `Mod The App` as you normally would with MBF.

## Repatching an Already Modded Game

You can set up the permission here:

![alt text](image-8.png)

Then click `Edit XML`. This process is the same as above — see the [Edit XML Guide](#edit-xml-guide).

After completing the XML patch, click the `Repatch Game` button.

![alt text](image-9.png)
