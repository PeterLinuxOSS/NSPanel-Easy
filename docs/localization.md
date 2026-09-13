# Localization

NSPanel Easy supports displaying dates, weekdays, and UI strings in your local language.
Language selection is configured at compile time in your ESPHome YAML, so only the strings for your chosen language
are loaded onto the device — saving memory and allowing the panel to display correct strings even when offline.

## Setting Your Language

Add the `language` substitution to your ESPHome YAML configuration:

```yaml
substitutions:
  device_name: "your_nspanel_name"
  friendly_name: "Your Panel Name"
  wifi_ssid: !secret wifi_ssid
  wifi_password: !secret wifi_password
  language: pt  # Set your language code here
```

The default language is `en` (English). If no `language` substitution is set, or if an unsupported code is provided,
English will be used as a fallback.

After changing the language, you must recompile and flash your panel for the change to take effect.

### Supported Languages

| Code | Language |
| :--- | :------- |
| `af` | Afrikaans |
| `ar` | Arabic |
| `bg` | Bulgarian |
| `bn` | Bengali |
| `bs` | Bosnian |
| `ca` | Catalan |
| `cs` | Czech |
| `cy` | Welsh |
| `da` | Danish |
| `de` | German |
| `el` | Greek |
| `en` | English *(default)* |
| `es` | Spanish |
| `et` | Estonian |
| `eu` | Basque |
| `fa` | Persian (Farsi) |
| `fi` | Finnish |
| `fr` | French |
| `gl` | Galician |
| `he` | Hebrew |
| `hi` | Hindi |
| `hr` | Croatian |
| `hu` | Hungarian |
| `id` | Indonesian |
| `is` | Icelandic |
| `it` | Italian |
| `ja` | Japanese |
| `jv` | Javanese |
| `ko` | Korean |
| `lt` | Lithuanian |
| `lv` | Latvian |
| `mk` | Macedonian |
| `ms` | Malay |
| `nb` | Norwegian Bokmål |
| `nl` | Dutch |
| `pa` | Punjabi (Gurmukhi) |
| `pl` | Polish |
| `pt` | Portuguese |
| `ro` | Romanian |
| `ru` | Russian |
| `sk` | Slovak |
| `sl` | Slovenian |
| `sq` | Albanian |
| `sr` | Serbian |
| `sv` | Swedish |
| `sw` | Swahili |
| `ta` | Tamil |
| `th` | Thai |
| `tr` | Turkish |
| `uk` | Ukrainian |
| `ur` | Urdu |
| `vi` | Vietnamese |
| `zh-cn` | Chinese (Simplified) |
| `zh-tw` | Chinese (Traditional) |

> [!NOTE]
> Languages using non-Latin scripts (Arabic, Bengali, Chinese, Cyrillic, Devanagari, Greek, Gurmukhi,
> Hebrew, Japanese, Korean, Persian, Tamil, Thai, Urdu) require a Nextion TFT font that supports the
> relevant character set. If your panel displays boxes or question marks instead of characters,
> your TFT file may not include the required font glyphs.

### Missing Your Language?

If your language is not listed, please [open a feature request](https://github.com/edwardtfn/NSPanel-Easy/issues/new)
and it will be added in a future release.

## Temperature Unit

The temperature unit is set in two places, and both must match.

- **Blueprint**: the "Temperature Unit" option under Localization. This applies to values the Blueprint renders and sends to the panel.
- **ESPHome**: the `temp_units` substitution in your panel's YAML. This applies to values the panel renders on its own, including while Home Assistant is unreachable.

```yaml
substitutions:
  temp_units: "°F"
```

Setting only one of them leaves the panel showing a mix of units.
