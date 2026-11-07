# Function renames: DIAL / VFO / MEM

* **DIAL** - the upper (0) or lower (1) radio block on the screen.
* **VFO** - free frequency selection.
* **MEM** - stored channels (memories).

Each dial is independently in VFO or MEM mode. `python3 apply_renames.py <project_dir>` applies this table (and the parameter renames such as `Vfo` -> `Dial`, `ChNo` -> `MemNo`) to any copy of the source.

| Old | New | Meaning |
|---|---|---|
| `CHANNELS_LoadChannel` | `MEM_LoadToDial` | Read a stored record into a dial. Returns true if it is empty/invalid. |
| `CHANNELS_SaveChannel` | `MEM_Save` | Write a dial state to a stored record. |
| `CHANNELS_GetChannelUp` | `MEM_LoadNextUp` | Next programmed MEM above, loaded into the dial; returns its number. |
| `CHANNELS_GetChannelDown` | `MEM_LoadNextDown` | Next programmed MEM below, loaded into the dial; returns its number. |
| `CHANNELS_CheckFreeChannels` | `MEM_CountStored` | Count programmed MEMs (gFreeChannelsCount); forces VFO mode if there is none. |
| `CHANNELS_GetDialChannel` | `DIAL_GetStoredNumber` | Stored number a dial points to: its MEM number, or its VFO slot 999 / 1000. |
| `CHANNELS_ReloadDial` | `DIAL_Reload` | Re-read what the dial points to, according to its mode. |
| `CHANNELS_LoadVfoModeDial` | `DIAL_LoadVfo` | Load the dial's VFO (restores the default VFO if the stored one is invalid). |
| `CHANNELS_LoadWorkModeDial` | `DIAL_LoadMem` | Load the dial's MEM (moves on if the stored MEM was deleted). |
| `CHANNELS_LoadDialMode` | `DIAL_Load` | Load one dial in its own mode (VFO or MEM). |
| `CHANNELS_NextChannelMr` | `DIAL_StepMem` | UP/DOWN step of the current dial in MEM mode. |
| `CHANNELS_NextChannelVfo` | `DIAL_StepVfo` | UP/DOWN step of the current dial in VFO mode. |
| `CHANNELS_UpdateChannel` | `DIAL_GoToTypedMem` | Go to the MEM number typed on the keypad. |
| `CHANNELS_UpdateVFO` | `DIAL_SetTypedVfoFrequency` | Set the VFO to the frequency typed on the keypad. |
| `CHANNELS_UpdateVFOFreq` | `DIAL_SetVfoFrequency` | Set the VFO of the current dial to a frequency. |
| `CHANNELS_SaveVfo` | `DIAL_SaveCurrent` | Save the current dial to its stored record (MEM or VFO slot). |
| `SETTINGS_GetWorkMode` | `DIAL_IsMemMode` | true if the dial is in MEM mode, false if in VFO mode. |
| `SETTINGS_SetWorkMode` | `DIAL_SetMemMode` | Put a dial in MEM mode (true) or VFO mode (false). |
| `SETTINGS_CurrentWorkMode` | `DIAL_CurrentIsMemMode` | Macro: DIAL_IsMemMode() of the selected dial. |
| `RADIO_Retune` | `DIAL_ReloadAndTune` | Reload the current dial from storage, then tune the radio. |
| `RADIO_SaveCurrentDial` | `DIAL_SaveAndTune` | Save the current dial to storage, then tune the radio. |
| `VFO_ClearCss` | `DIAL_ClearCss` | Clear tone / DCS / encryption of the current dial. |
| `VFO_ClearMute` | `DIAL_ClearMute` | Clear mute settings of the current dial. |
| `RADIO_DrawChannelMode` | `DIAL_DrawMemMode` | Redraw the current dial after leaving MEM-number entry. |
| `RADIO_DrawFrequencyMode` | `DIAL_DrawVfoMode` | Redraw the frequency of the current dial after leaving frequency entry. |
| `JumpToVFO` | `SwitchCurrentDialToVfo` | Spectrum: put the current dial in VFO mode on the chosen frequency. |
| `CHANNEL_AppendDigit` | `AppendDigitToMem` | Keypad: digit typed while the dial is in MEM mode. |
| `VFO_AppendDigit` | `AppendDigitToVfo` | Keypad: digit typed while the dial is in VFO mode. |
| `CHANNEL_KeyHandler` | `MEM_KeyHandler` | Menu: key handler of the MEM (save / delete channel) editor. |
| `DrawEditChannel` | `DrawEditMem` | Menu: draw the MEM editor. |
| `AUDIO_PlayChannelNumber` | `AUDIO_PlayMemNumber` | Voice prompt: announce the MEM number. |
| `APRS_restore_Dial` | `APRS_RestoreDial` | Put the dial back after the APRS transmission. |
| `UI_DrawChannel` | `UI_DrawDialModeLabel` | Dial label: "VFO" or "CHnnn". |
| `UI_DrawName` | `UI_DrawDialMemName` | Dial: MEM name. |
| `UI_DrawChannelNumber` | `UI_DrawMenuMemNumber` | Menu: MEM number being typed. |
| `UI_DrawChannelName` | `UI_DrawMenuMemName` | Menu: MEM name. |
| `UI_DrawVfoFrame` | `UI_DrawDialBarFrame` | Dial: frame of the RX/TX signal bar. |
| `UI_DrawFrequency` | `UI_DrawDialFrequency` | Dial: frequency. |
| `UI_DrawFrequencyEx` | `UI_DrawDialFrequencyText` | Dial: frequency from a text string. |
| `UI_DrawDigits` | `UI_DrawDialTypedDigits` | Dial: digits being typed on the keypad. |
| `UI_DrawRegisters` | `UI_DrawDialRegisters` | Dial: RF register view (single-display mode). |
| `UI_DrawModType` | `UI_DrawDialModulation` | Dial: modulation indicator. |
| `UI_DrawCss` | `UI_DrawDialCss` | Dial: tone / DCS / mute / encryption indicator. |
| `UI_DrawTxPower` | `UI_DrawDialTxPower` | Dial: TX power indicator. |
| `UI_DrawRxCTCSS` | `UI_DrawDialRxCtcss` | Dial: received CTCSS tone. |
| `UI_DrawRxDBM` | `UI_DrawDialRxDbm` | Dial: received level in dBm. |
| `UI_DrawRX` | `UI_DrawDialAntennaIcon` | Dial: antenna (RX/TX) icon. |
| `UI_DrawBar` | `UI_DrawDialSignalBar` | Dial: signal bar. |
| `UI_DrawMainBitmap` | `UI_DrawDialMainMarker` | Dial: "MAIN" marker of the selected dial. |
| `UI_DrawScanLists` | `UI_DrawDialScanLists` | Dial: scan-list row. |
| `DrawBandwidth` | `DrawDialBandwidth` | Dial: bandwidth indicator (ui/vfo.c). |
| `DrawRadio` | `DrawDialRadioIcon` | Dial: radio icon (ui/helper.c). |

Not renamed on purpose: `CHANNELS_NextFM`, `CHANNELS_NextNOAA`, `CHANNELS_SetNoaaChannel` (FM / NOAA, not dial concepts), `RADIO_Tune` (its argument also selects NOAA and APRS), and variables / types such as `gVfoState`, `gVfoInfo`, `gFreeChannelsCount`, `ChannelInfo_t`.
