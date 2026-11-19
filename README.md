# NEUROFORGE Alpha 0.1

An experimental JUCE-based VST3 audio effect for multiband bass mutation.

## What this first build contains

- VST3 and Standalone targets
- Stereo and mono audio effect routing
- Three-band split using Linkwitz-Riley crossover filters
- Eight waveshaping/distortion characters
- Independent mid/high distortion character
- Movement-driven drive modulation
- Tone control
- Mutation personalities: Audio, Ed Rush, Alien, Destruction
- MUTATE / EVOLVE / VARIATION controls
- Eight parameter locks
- Host automation and state recall
- Mix and output controls

This is an **Alpha 0.1 source project**, not a precompiled VST3 binary. It is a first playable foundation, not yet the complete specification. Spectral processing, a full modulation matrix, advanced multiband routing, oversampling controls and a dedicated sub guardian remain future milestones.

## Requirements (Windows 10/11)

1. Visual Studio 2022 with the **Desktop development with C++** workload.
2. CMake 3.22 or newer.
3. Git for Windows.
4. Internet access for the first CMake configure: JUCE is downloaded from its official GitHub repository.
5. Ableton Live with VST3 support.

## Build

Open **x64 Native Tools Command Prompt for VS 2022** or PowerShell with the VS build tools available.

From this folder:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The first configure downloads JUCE 8.0.6. This can take a few minutes.

The VST3 should be generated under a path similar to:

```text
build/Neuroforge_artefacts/Release/VST3/Neuroforge.vst3
```

Exact artifact paths can vary slightly by JUCE/CMake version.

## Install in Ableton Live

1. Copy `Neuroforge.vst3` to:
   `%COMMONPROGRAMFILES%\\VST3`
   (usually `C:\\Program Files\\Common Files\\VST3`).
2. Open Ableton Live.
3. Open Preferences → Plug-Ins.
4. Ensure VST3 System Folders is enabled.
5. Rescan plug-ins.
6. Find **Neuroforge** under Audio Effects → VST3.

## First sound-design test

1. Create a MIDI track and load Serum.
2. Make a simple saw or reese bass patch.
3. Put Neuroforge after Serum.
4. Start with Mix at 100%, Drive around 8–14 dB, Movement around 20–40%, Metal around 15–35%.
5. Try AUDIO, ED RUSH, ALIEN and DESTRUCTION personalities.
6. Press MUTATE and compare results at matched loudness.
7. Use the LOCK buttons to preserve parts you like.

Keep your monitoring level low when exploring extreme distortion.

## Important implementation notes

- The first alpha uses Linkwitz-Riley crossover filters and a small number of modulation behaviors.
- The mutation engine currently changes a focused set of parameters. Its personality choices are starting points, not trained models or exact artist emulations.
- The lock controls and parameter state are currently simple prototype behavior; lock states are not yet serialized with presets.
- The current UI is intentionally functional rather than a final commercial design.
- Please test at conservative levels. Some distortion algorithms can produce high peak levels.

## Troubleshooting

**CMake cannot find Visual Studio:** install Visual Studio 2022 and the Desktop development with C++ workload, then rerun the configure command.

**JUCE download fails:** check internet access and rerun the CMake configure command.

**Plugin does not appear in Ableton:** confirm the `.vst3` bundle is in the system VST3 folder, enable VST3 system folders, then rescan.

**Build errors:** save the complete CMake/build output. Compiler messages are useful for identifying version-specific API changes.

## Licensing

JUCE is a separate third-party dependency. Review the JUCE licensing terms at https://juce.com/legal/ before distributing or selling a plugin built with it. This project does not include the JUCE source code.


## Build a Windows VST3 without installing a compiler locally

This repository includes a GitHub Actions workflow at `.github/workflows/build-windows.yml`.

1. Create a GitHub account if you do not already have one.
2. Create a new **private** repository.
3. Upload the contents of this project (including the hidden `.github` folder) to the repository.
4. Open the repository's **Actions** tab.
5. Select **Build Neuroforge VST3 (Windows)** and choose **Run workflow**.
6. When the run completes successfully, open it and download the `Neuroforge-Windows-VST3` artifact.
7. Extract the artifact. Copy the `Neuroforge.vst3` folder into `C:\\Program Files\\Common Files\\VST3`.
8. Rescan VST3 plug-ins in Ableton Live.

This workflow builds on a Windows 2022 runner. It does not mean the plugin has already been compiled or tested; the artifact only exists after a successful workflow run.
