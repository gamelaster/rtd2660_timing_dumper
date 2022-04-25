#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct
{
  uint8_t PanelStyle;                 // Panel Style
  uint8_t PanelConfig;                // Panel Configuration

  uint16_t DHStartPos;                // Display Horizontal Start Position
  uint16_t DHWidth;                   // Display Horizontal Width

  uint16_t DHTotal;                   // Display Horizontal Total Clock Number in One Display Line
  uint16_t PalDHTotal;                // Display Horizontal Total Clock Number in One Display Line for CVBS PAL
  uint16_t NtscDHTotal;               // Display Horizontal Total Clock Number in One Display Line for CVBS NTSC

  uint16_t DVStartPos;                // Display Vertical Start Position
  uint16_t DVHeight;                  // Display Vertical Height

  uint16_t DVTotal;                   // Display Vertical Total Line Number in One Frame

  uint8_t DHSyncWidth;                // Display H Sync Width
  uint8_t DVSyncHeight;               // Display V Sync Height

  uint16_t PixelClock;                // Typical Pixel Clock in MHz

  uint16_t HSyncMaxFreq;              // H Sync Max Freq Unit in 0.1 kHZ
  uint16_t HSyncMinFreq;              // H Sync Min Freq Unit in 0.1 kHZ
  uint16_t VSyncMaxFreq;              // V Sync Max Freq Unit in 0.1 HZ
  uint16_t VSyncMinFreq;              // V Sync Min Freq Unit in 0.1 HZ

  uint8_t TTL20;

} PanelType;
#pragma pack(pop)


uint16_t uint16_endianness_swap(uint16_t input)
{
  return ((input >> 8) & 0xFF) | ((input & 0xFF) << 8);
}

void print_panel(PanelType* panelType)
{
  printf("PanelStyle: %d      // Panel Style\n", panelType->PanelStyle);
  printf("PanelConfig: %d     // Panel Configuration\n", panelType->PanelConfig);
  printf("DHStartPos: %d      // Display Horizontal Start Position\n", panelType->DHStartPos);
  printf("DHWidth: %d         // Display Horizontal Width\n", panelType->DHWidth);
  printf("DHTotal: %d         // Display Horizontal Total Clock Number in One Display Line\n", panelType->DHTotal);
  printf("PalDHTotal: %d      // Display Horizontal Total Clock Number in One Display Line for CVBS PAL\n", panelType->PalDHTotal);
  printf("NtscDHTotal: %d     // Display Horizontal Total Clock Number in One Display Line for CVBS NTSC\n", panelType->NtscDHTotal);
  printf("DVStartPos: %d      // Display Vertical Start Position\n", panelType->DVStartPos);
  printf("DVHeight: %d        // Display Vertical Height\n", panelType->DVHeight);
  printf("DVTotal: %d         // Display Vertical Total Line Number in One Frame\n", panelType->DVTotal);
  printf("DHSyncWidth: %d     // Display H Sync Width\n", panelType->DHSyncWidth);
  printf("DVSyncHeight: %d    // Display V Sync Height\n", panelType->DVSyncHeight);
  printf("PixelClock: %d      // Typical Pixel Clock in MHz\n", panelType->PixelClock);
  printf("HSyncMaxFreq: %d    // H Sync Max Freq Unit in 0.1 kHZ\n", panelType->HSyncMaxFreq);
  printf("HSyncMinFreq: %d    // H Sync Min Freq Unit in 0.1 kHZ\n", panelType->HSyncMinFreq);
  printf("VSyncMaxFreq: %d    // V Sync Max Freq Unit in 0.1 HZ\n", panelType->VSyncMaxFreq);
  printf("VSyncMinFreq: %d    // V Sync Min Freq Unit in 0.1 HZ\n", panelType->VSyncMinFreq);
}

int main(int argc, char** argv)
{
  if (argc < 4) {
    fprintf(stderr, "Usage: <path_to_firmware_dump_bin> <panel_width> <panel_height>\n");
    return -1;
  }
  uint16_t panel_width = atoi(argv[2]);
  uint16_t panel_height = atoi(argv[3]);
  puts("Searching for the timings...\n");
  fflush(stdout);

  uint16_t panel_width_be = uint16_endianness_swap(panel_width);
  uint8_t* panel_width_be_p = (uint8_t*)&panel_width_be;

  FILE* firmware_fd = fopen(argv[1], "rb");

  fseek(firmware_fd, 0L, SEEK_END);
  uint64_t firmware_size = ftell(firmware_fd);
  rewind(firmware_fd);

  uint8_t* firmware = malloc(firmware_size);

  fread(firmware, 1, firmware_size, firmware_fd);

  uint8_t found = 0;
  uint64_t offset = 0;
  while (offset < firmware_size) {
    if ((offset + 1) >= firmware_size) {
      continue;
    }
    if (firmware[offset] == panel_width_be_p[0] && firmware[offset + 1] == panel_width_be_p[1]) {
      uint8_t found2 = 1;
      uint16_t previous_number;
      for (uint8_t i = 0; i < 3; i++) {
        uint64_t number_offset = offset + ((i + 1) * 2);
        uint16_t number = ((firmware[number_offset + 0] & 0xFF) << 8) | firmware[number_offset + 1];
        if (i > 0) {
          if (abs(previous_number - number) > 50) {
            found2 = 0;
            break;
          }
        }
        previous_number = number;
      }
      if (found2) {
        found = 1;
        break;
      }
    }
    offset++;
  }

  if (!found) {
    fprintf(stderr, "Failed to find the timings.\n");
    return -1;
  }

  PanelType panel = {0};
  memcpy(&panel, firmware + offset - 4, 31);

  panel.DHStartPos = uint16_endianness_swap(panel.DHStartPos);
  panel.DHWidth = uint16_endianness_swap(panel.DHWidth);
  panel.DHTotal = uint16_endianness_swap(panel.DHTotal);
  panel.PalDHTotal = uint16_endianness_swap(panel.PalDHTotal);
  panel.NtscDHTotal = uint16_endianness_swap(panel.NtscDHTotal);
  panel.DVStartPos = uint16_endianness_swap(panel.DVStartPos);
  panel.DVHeight = uint16_endianness_swap(panel.DVHeight);
  panel.DVTotal = uint16_endianness_swap(panel.DVTotal);
  panel.PixelClock = uint16_endianness_swap(panel.PixelClock);
  panel.HSyncMaxFreq = uint16_endianness_swap(panel.HSyncMaxFreq);
  panel.HSyncMinFreq = uint16_endianness_swap(panel.HSyncMinFreq);
  panel.VSyncMaxFreq = uint16_endianness_swap(panel.VSyncMaxFreq);
  panel.VSyncMinFreq = uint16_endianness_swap(panel.VSyncMinFreq);

  if (panel.DVHeight != panel_height) {
    fprintf(stderr, "Failed to find the timings.\n");
    return -1;
  }

  print_panel(&panel);

  free(firmware);
  fclose(firmware_fd);

  return 0;
}
