# rtd2660_timing_dumper
LVDS / TTL Timing dumper from firmwares for RTD2660 / RTD2662

# Usage:

`rtd2660_timing_dumper <path_to_firmware_dump_bin> <panel_width> <panel_height>`

# How it works

The principle is very simple, but not reliable every time. The app searchs for `panel_width` (in Big-Endian),
if it will find it, it looks for next three `uint16_t` (DHTotal, PalDHTotal and NtscDHTotal). Those timings
are mostly same or -+50 same. If this is true, we know this is `PanelType` structure and parse it.
If the `DVHeight` is same as supplied `panel_height`, that's last confirmation of validity of the finding.

Also, to be even more sure, those data should be within `0x0000 - 0x1500` of firmware, since those structures
should be within ICODE segment, which is located at very early beginning of the firmware. Secondly, this structure
is two times within firmware, the second time it's in second bank, located after `0xFFFF`.
