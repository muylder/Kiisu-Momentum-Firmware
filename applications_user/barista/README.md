# Barista

Barista is a manual coffee companion for Flipper Zero and Kiisu. It includes editable profiles for Espresso, V60, AeroPress, French press, Chemex, Kalita Wave, Clever Dripper, Moka pot, Cold Brew, Espresso with pre-infusion and a Custom method, a guided timer, target-water calculation from dose and ratio, up to eight configurable stages per recipe, stage prompts and a 12-entry tasting journal. Cold Brew recipes support long infusions up to 18 hours.

The initial coffee library is documented in [RECIPES.md](RECIPES.md), with classic espresso, V60 4:6, inverted AeroPress, Chemex, Kalita, Clever, moka and cold brew profiles.

The app does not read a scale or temperature probe. Dose, ratio, temperature and target water are preparation targets; use a real scale and thermometer for measurements.

## Controls

- Home: Up/Down selects a method, OK opens its recipe, Right opens the journal, Left opens the guide.
- Recipe: Up/Down selects a field, Left/Right adjusts it, OK advances or starts the brew.
- Recipe: the `Porcoes` field scales dose and cumulative water targets while preserving each step's timing and temperature.
- Recipe: `Moagem` stores a variable grinder reference from 1 (fine) to 100 (coarse), with a fine/medium/coarse band shown for repeatable dialing-in.
- Guide → page 2 → OK opens the grinder setup. Set the minimum and maximum click counts of your grinder; each recipe's 1–100 setting is mapped into that range and shown in clicks.
- Recipe → Edit steps: each of up to eight stages has an editable duration and cumulative water target; OK advances through the stages and returns to the recipe.
- Brew: OK pauses/resumes, Right finishes, Back opens the discard confirmation.
- Result: Left/Right changes the 1–5 rating, OK saves the brew to the journal.
- Result: Up/Down selects yield, overall rating, acidity, body, sweetness, bitterness or finish; Left/Right adjusts the selected value from 0–5. The app shows the resulting brew ratio.
- Guide page 3: OK switches between Português and English. The language is saved on the SD card.

The app stores two alternating snapshots under `apps_data/barista/`; a failed write keeps the previous snapshot available. If the SD card is unavailable, the app continues in memory and marks the journal as unavailable. Existing snapshots from the earlier pre-stage format are rejected safely and replaced by the new defaults.

This app is authored in this fork by muylder with AI-assisted implementation. It is licensed with the firmware under GPL-3.0-or-later.

## Build the test FAP on Windows

From the repository root in PowerShell:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\build_barista.ps1
```

The script uses the repository's bundled Python, SCons and ARM toolchain and prints the generated `.fap` path under `dist/`. Use `.\build_barista.ps1 -Clean` to rebuild from clean app intermediates.
