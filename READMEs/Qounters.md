# Supported Version

This mod supports [Qounters++](https://github.com/Metalit/Qounters/tree/main).

The Qounters++ feature is enabled in the following versions:

| Game Version | HeartBeatQuest Version | Qounters++ Version | Notes |
|--------------|------------------------|--------------------|-------|
| `1.40.8`     | `>=TBD`               | N/A                | |
| `<1.40.8`    | N/A                   | N/A                | Qounters++ with the API isn't available on older game versions |

# Compatibility

The Qounters++ feature is enabled automatically when a compatible Qounters++ mod is detected.

When Qounters++ is detected, the built-in UI (which is what you see without Qounters++) will be disabled automatically.

# Qounters++ Support

Add the heart rate manually in the Qounters++ configuration. The following sources are supported:

## Text Source

A text source called `HeartRate` is added to Qounters++. It displays your heart rate number with an optional heart icon as a prefix or suffix.

## Shape Source

A float source called `HeartRatePercent` is added to Qounters++.

The float value is calculated from your configured max heart rate. This is useful for creating shapes that indicate your heart rate zone.

By default, it outputs a value aligned to the following table:

| HeartRate / MaxHeartRate | Aligned Output |
|--------------------------|----------------|
| `<50%`                   | 0%             |
| `50%–60%`                | 20%            |
| `60%–70%`                | 40%            |
| `70%–80%`                | 60%            |
| `80%–90%`                | 80%            |
| `>90%`                   | 100%           |

You can also configure it to output the raw `HeartRate / MaxHeartRate` value directly.

## Enable Source

A boolean source called `HeartRatePercentRange` is added to Qounters++.

You can configure it to output `true` or `false` per heart rate zone. This is useful for displaying something only in one or more specific zones.

## Color Source

A color source called `HeartRateRangeColor` is added to Qounters++.

You can configure it to output different colors per heart rate zone. This is useful for changing UI element colors based on the current zone.

# Manual Compatibility

If you disable Qounters++ in the game menu, HeartBeatQuest will not load its UI. Enable the `Ignore Qounters++ mod` option in the HeartBeat settings menu to make the mod work on its own.

Do **not** enable that option when you're actually using Qounters++.
