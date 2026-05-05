#ifdef WITH_QOUNTERS

#include "QountersDriver.hpp"
#include "ModConfig.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "bsml/shared/BSML-Lite/Creation/Misc.hpp"
#include "bsml/shared/BSML-Lite/Creation/Settings.hpp"
#include "bsml/shared/BSML/Components/Settings/ColorSetting.hpp"
#include "main.hpp"
#include "rapidjson-macros/shared/macros.hpp"
#include <dlfcn.h>
#include "metacore/shared/events.hpp"


namespace HeartBeat {
namespace Qounters {

#define METACORE_EVENT_MOD "HeartBeatQuest"
#define METACORE_EVENT_ID 1

bool (*Editor_FinalizeAction)() = nullptr;
int (*Editor_GetActionId)() = nullptr;
void (*Editor_SetSourceOptions)(int actionId, UnparsedJSON options) = nullptr;
void (*Events_RegisterToEvent)(::Qounters::Types::Sources sourceType, std::string source, std::string mod,
                               int event) = nullptr;
void (*Editor_SetColorOptions)(int actionId, UnparsedJSON options) = nullptr;
void (*SetEnableOptions)(int actionId, UnparsedJSON options) = nullptr;

BSML::ColorSetting* (*CreateColorPicker)(UnityEngine::GameObject* parent, std::string name, UnityEngine::Color value,
                                         std::function<void(UnityEngine::Color)> onChange,
                                         std::function<void()> onClose);

static int qounters_hr = 0;

DECLARE_JSON_STRUCT(HeartRateTextOption) {
    VALUE_DEFAULT(bool, heartBeforeText, false);
    VALUE_DEFAULT(bool, heartAfterText, false);
};

std::string hrTextSource(UnparsedJSON unparsed) {
    static HeartRateTextOption opts;
    opts = unparsed.Parse<HeartRateTextOption>();

    char buff[128];
    sprintf(buff,
            opts.heartBeforeText  ? opts.heartAfterText ? "♥ %d♥" : "♥ %d"
            : opts.heartAfterText ? "%d♥"
                                  : "%d",
            qounters_hr);
    return buff;
}

void hrTextSourceUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static HeartRateTextOption opts;
    opts = unparsed.Parse<HeartRateTextOption>();
    BSML::Lite::CreateToggle(parent, "Heart Icon Before Text", opts.heartBeforeText, [](bool v) {
        static int id = Editor_GetActionId();
        opts.heartBeforeText = v;
        Editor_SetSourceOptions(id, opts);
        Editor_FinalizeAction();
    });
    BSML::Lite::CreateToggle(parent, "Heart Icon After Text", opts.heartAfterText, [](bool v) {
        static int id = Editor_GetActionId();
        opts.heartAfterText = v;
        Editor_SetSourceOptions(id, opts);
        Editor_FinalizeAction();
    });
}

DECLARE_JSON_STRUCT(HeartRatePercentOption) {
    VALUE_DEFAULT(bool, alignedTo5Range, true);
};

float hrPercentSource(UnparsedJSON unparsed) {
    static HeartRatePercentOption opts;
    opts = unparsed.Parse<HeartRatePercentOption>();

    int Maximum = getModConfig().MaxHeart.GetValue();
    float percent = ((float)qounters_hr) / Maximum;

    if (opts.alignedTo5Range) {
        /*
        50% - 60% : 1
        60% - 70% : 2
        70% - 80% : 3
        80% - 90% : 4
        90% - 100%: 5
        */
        int level = percent * 10 - 4;
        if (level < 0)
            level = 0;
        if (level > 5)
            level = 5;

        percent = ((float)level) / 5;
    }

    return percent;
}

void hrPercentSourceUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static HeartRatePercentOption opts;
    opts = unparsed.Parse<HeartRatePercentOption>();
    BSML::Lite::CreateToggle(parent, "Align to 5 level range", opts.alignedTo5Range, [](bool v) {
        static int id = Editor_GetActionId();
        opts.alignedTo5Range = v;
        Editor_SetSourceOptions(id, opts);
        Editor_FinalizeAction();
    });
}

DECLARE_JSON_STRUCT(HeartRateRangeEnableOption) {
    VALUE_DEFAULT(bool, range__5, false);
    VALUE_DEFAULT(bool, range_5_6, false);
    VALUE_DEFAULT(bool, range_6_7, false);
    VALUE_DEFAULT(bool, range_7_8, false);
    VALUE_DEFAULT(bool, range_8_9, false);
    VALUE_DEFAULT(bool, range_9_, false);
};

bool hrRangeEnable(UnparsedJSON unparsed) {
    static HeartRateRangeEnableOption opts;
    opts = unparsed.Parse<HeartRateRangeEnableOption>();

    int Maximum = getModConfig().MaxHeart.GetValue();
    float percent = ((float)qounters_hr) / Maximum;
    if (percent < 0.5)
        return opts.range__5;
    if (percent < 0.6)
        return opts.range_5_6;
    if (percent < 0.7)
        return opts.range_6_7;
    if (percent < 0.8)
        return opts.range_7_8;
    if (percent < 0.9)
        return opts.range_8_9;
    return opts.range_9_;
}
void hrRangeEnableUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static HeartRateRangeEnableOption opts;
    opts = unparsed.Parse<HeartRateRangeEnableOption>();
    BSML::Lite::CreateToggle(parent, "Enable if range lower than 50%", opts.range__5, [](bool v) {
        static int id = Editor_GetActionId();
        opts.range__5 = v;
        SetEnableOptions(id, opts);
        Editor_FinalizeAction();
    });
    BSML::Lite::CreateToggle(parent, "Enable if range in 50% - 60%", opts.range_5_6, [](bool v) {
        static int id = Editor_GetActionId();
        opts.range_5_6 = v;
        SetEnableOptions(id, opts);
        Editor_FinalizeAction();
    });
    BSML::Lite::CreateToggle(parent, "Enable if range in 60% - 70%", opts.range_6_7, [](bool v) {
        static int id = Editor_GetActionId();
        opts.range_6_7 = v;
        SetEnableOptions(id, opts);
        Editor_FinalizeAction();
    });
    BSML::Lite::CreateToggle(parent, "Enable if range in 70% - 80%", opts.range_7_8, [](bool v) {
        static int id = Editor_GetActionId();
        opts.range_7_8 = v;
        SetEnableOptions(id, opts);
        Editor_FinalizeAction();
    });
    BSML::Lite::CreateToggle(parent, "Enable if range in 80% - 90%", opts.range_8_9, [](bool v) {
        static int id = Editor_GetActionId();
        opts.range_8_9 = v;
        SetEnableOptions(id, opts);
        Editor_FinalizeAction();
    });
    BSML::Lite::CreateToggle(parent, "Enable if range larger than 90%", opts.range_9_, [](bool v) {
        static int id = Editor_GetActionId();
        opts.range_9_ = v;
        SetEnableOptions(id, opts);
        Editor_FinalizeAction();
    });
}

DECLARE_JSON_STRUCT(HeartRateRangeColorOption) {
    VALUE_DEFAULT(ConfigUtils::Color, range__5, ConfigUtils::Color(1, 1, 1, 1));
    VALUE_DEFAULT(ConfigUtils::Color, range_5_6, ConfigUtils::Color(1, 1, 1, 1));
    VALUE_DEFAULT(ConfigUtils::Color, range_6_7, ConfigUtils::Color(1, 1, 1, 1));
    VALUE_DEFAULT(ConfigUtils::Color, range_7_8, ConfigUtils::Color(1, 1, 1, 1));
    VALUE_DEFAULT(ConfigUtils::Color, range_8_9, ConfigUtils::Color(1, 1, 1, 1));
    VALUE_DEFAULT(ConfigUtils::Color, range_9_, ConfigUtils::Color(1, 1, 1, 1));
};

UnityEngine::Color hrRangeColor(UnparsedJSON unparsed) {
    static HeartRateRangeColorOption opts;
    opts = unparsed.Parse<HeartRateRangeColorOption>();

    int Maximum = getModConfig().MaxHeart.GetValue();
    float percent = ((float)qounters_hr) / Maximum;
    if (percent < 0.5)
        return opts.range__5;
    if (percent < 0.6)
        return opts.range_5_6;
    if (percent < 0.7)
        return opts.range_6_7;
    if (percent < 0.8)
        return opts.range_7_8;
    if (percent < 0.9)
        return opts.range_8_9;
    return opts.range_9_;
}

void hrRangeColorUI(UnityEngine::GameObject* parent, UnparsedJSON unparsed) {
    static HeartRateRangeColorOption opts;
    opts = unparsed.Parse<HeartRateRangeColorOption>();

    BSML::ColorSetting* sptr;
    sptr = CreateColorPicker(
        parent, "Color when lower than 50%", opts.range__5,
        [](UnityEngine::Color val) {
            static int id = Editor_GetActionId();
            opts.range__5 = val;
            Editor_SetColorOptions(id, opts);
        },
        Editor_FinalizeAction);
    BSML::Lite::AddHoverHint(sptr, "Pickup a color");
    sptr = CreateColorPicker(
        parent, "Color when range in 50% - 60%", opts.range_5_6,
        [](UnityEngine::Color val) {
            static int id = Editor_GetActionId();
            opts.range_5_6 = val;
            Editor_SetColorOptions(id, opts);
        },
        Editor_FinalizeAction);
    BSML::Lite::AddHoverHint(sptr, "Pickup a color");
    sptr = CreateColorPicker(
        parent, "Color when range in 60% - 70%", opts.range_6_7,
        [](UnityEngine::Color val) {
            static int id = Editor_GetActionId();
            opts.range_6_7 = val;
            Editor_SetColorOptions(id, opts);
        },
        Editor_FinalizeAction);
    BSML::Lite::AddHoverHint(sptr, "Pickup a color");
    sptr = CreateColorPicker(
        parent, "Color when range in 70% - 80%", opts.range_7_8,
        [](UnityEngine::Color val) {
            static int id = Editor_GetActionId();
            opts.range_7_8 = val;
            Editor_SetColorOptions(id, opts);
        },
        Editor_FinalizeAction);
    BSML::Lite::AddHoverHint(sptr, "Pickup a color");
    sptr = CreateColorPicker(
        parent, "Color when range in 80% - 90%", opts.range_8_9,
        [](UnityEngine::Color val) {
            static int id = Editor_GetActionId();
            opts.range_8_9 = val;
            Editor_SetColorOptions(id, opts);
        },
        Editor_FinalizeAction);
    BSML::Lite::AddHoverHint(sptr, "Pickup a color");
    sptr = CreateColorPicker(
        parent, "Color when range more than 90%", opts.range_9_,
        [](UnityEngine::Color val) {
            static int id = Editor_GetActionId();
            opts.range_9_ = val;
            Editor_SetColorOptions(id, opts);
        },
        Editor_FinalizeAction);
    BSML::Lite::AddHoverHint(sptr, "Pickup a color");
}


void DisplayData(int heartrate) {
    qounters_hr = heartrate;
    MetaCore::Events::Broadcast(METACORE_EVENT_MOD, METACORE_EVENT_ID);
}

static bool enabled = false;

void Init() {
    for (auto& mod : modloader::get_loaded()) {
        if (mod.info.id == "Qounters++" && mod.info.version.starts_with("1.1.")) {
            auto texts = (::Qounters::Sources::TextsTy*)dlsym(mod.handle, "_ZN8Qounters7Sources5textsE");
            auto shapes = (::Qounters::Sources::ShapesTy*)dlsym(mod.handle, "_ZN8Qounters7Sources6shapesE");
            auto colors = (::Qounters::Sources::ColorsTy*)dlsym(mod.handle, "_ZN8Qounters7Sources6colorsE");
            auto enables = (::Qounters::Sources::EnablesTy*)dlsym(mod.handle, "_ZN8Qounters7Sources7enablesE");

            Editor_FinalizeAction =
                (decltype(Editor_FinalizeAction))dlsym(mod.handle, "_ZN8Qounters3API14FinalizeActionEv");
            Events_RegisterToEvent = (decltype(Events_RegisterToEvent))dlsym(
                mod.handle, "_ZN8Qounters6Events15RegisterToEventENS_5Types7SourcesENSt6__ndk112basic_stringIcNS3_"
                            "11char_traitsIcEENS3_9allocatorIcEEEES9_i");
            Editor_GetActionId = (decltype(Editor_GetActionId))dlsym(mod.handle, "_ZN8Qounters3API11GetActionIdEv");
            Editor_SetSourceOptions = (decltype(Editor_SetSourceOptions))dlsym(
                mod.handle, "_ZN8Qounters3API16SetSourceOptionsEi12UnparsedJSON");
            Editor_SetColorOptions = (decltype(Editor_SetColorOptions))dlsym(
                mod.handle, "_ZN8Qounters3API15SetColorOptionsEi12UnparsedJSON");
            SetEnableOptions =
                (decltype(SetEnableOptions))dlsym(mod.handle, "_ZN8Qounters6Editor16SetEnableOptionsEi12UnparsedJSON");
            CreateColorPicker = (decltype(CreateColorPicker))dlsym(
                mod.handle,
                "_ZN8Qounters3API17CreateColorPickerEPN11UnityEngine10GameObjectENSt6__ndk112basic_stringIcNS4_11char_"
                "traitsIcEENS4_9allocatorIcEEEENS1_5ColorENS4_8functionIFvSB_EEENSC_IFvvEEE");


            if (texts && Events_RegisterToEvent && Editor_FinalizeAction && Editor_GetActionId &&
                Editor_SetSourceOptions && Editor_SetColorOptions) {
                getLogger().info("Qounters {} detected, will load texts.", mod.info.version);

                ::Qounters::Sources::Register(*texts, "HeartRate",
                                              ::Qounters::Types::SourceFn<std::string>(hrTextSource), hrTextSourceUI);
                ::Qounters::Sources::Register(*shapes, "HeartRatePrecent",
                                              ::Qounters::Types::SourceFn<float>(hrPercentSource), hrPercentSourceUI);
                ::Qounters::Sources::Register(*enables, "HeartRatePercentRange",
                                              ::Qounters::Types::SourceFn<bool>(hrRangeEnable), hrRangeEnableUI);
                ::Qounters::Sources::Register(*colors, "HeartRateRangeColor",
                                              ::Qounters::Types::SourceFn<UnityEngine::Color>(hrRangeColor),
                                              hrRangeColorUI);

                MetaCore::Events::RegisterEvent(METACORE_EVENT_MOD, METACORE_EVENT_ID);
                Events_RegisterToEvent(::Qounters::Types::Sources::Text, "HeartRate", METACORE_EVENT_MOD,
                                       METACORE_EVENT_ID);
                Events_RegisterToEvent(::Qounters::Types::Sources::Shape, "HeartRatePrecent", METACORE_EVENT_MOD,
                                       METACORE_EVENT_ID);
                Events_RegisterToEvent(::Qounters::Types::Sources::Enable, "HeartRatePercentRange", METACORE_EVENT_MOD,
                                       METACORE_EVENT_ID);
                Events_RegisterToEvent(::Qounters::Types::Sources::Color, "HeartRateRangeColor", METACORE_EVENT_MOD,
                                       METACORE_EVENT_ID);
                enabled = true;
                return;
            }
        }
    }
    getLogger().info("Qounters not detected.");
}

bool Enabled() {
    return enabled;
}

} // namespace Qounters
} // namespace HeartBeat

#endif // WITH_QOUNTERS
