
ShuckCMD meminfo("meminfo", "Show memory info", [](ShuckArgs& args, ShuckConsole& console) -> int {

    uint32_t free = ESP.getFreeHeap() / 1024;
    uint32_t total = ESP.getHeapSize() / 1024;
    uint32_t used = total - free;
    uint32_t min = ESP.getMinFreeHeap() / 1024;

    console.user.printf("Heap: %u KB free, %u KB used, (%u KB total)\n", free, used, total);
    console.user.printf("Minimum free heap size during uptime was: %u KB\n", min);

    return SHUCK_CMD_SUCCESS;
});

ShuckCMD restart("restart", "Restart ESP", [](ShuckArgs& args, ShuckConsole& console) -> int {

    ESP.restart();
    
    return SHUCK_CMD_SUCCESS;
});
/*
ShuckCMD espinfo("espinfo", "Show ESP32 info", [](ShuckArgs& args, ShuckPrompt& cl) {
    auto mac2String = [](uint64_t mac) -> String {
        byte *ar = (byte *)&mac;
        String s;
        for (byte i = 0; i < 6; ++i)
        {
            char buf[3];
            sprintf(buf, "%02X", ar[i]); // J-M-L: slight modification, added the 0 in the format for padding
            s += buf;
            if (i < 5)
                s += ':';
        }
        return s;
    };
    auto getFlashModeStr = []() -> const char* {
        switch (ESP.getFlashChipMode())
        {
        case FM_DIO:
            return "DIO";
        case FM_DOUT:
            return "DOUT";
        case FM_FAST_READ:
            return "FAST READ";
        case FM_QIO:
            return "QIO";
        case FM_QOUT:
            return "QOUT";
        case FM_SLOW_READ:
            return "SLOW READ";
        case FM_UNKNOWN:
        default:
            return "UNKNOWN";
        }
    };
    
    auto getResetReasonStr = []() -> const char* {
        switch (esp_reset_reason())
        {
        case ESP_RST_BROWNOUT:
            return "Brownout reset (software or hardware)";
        case ESP_RST_DEEPSLEEP:
            return "Reset after exiting deep sleep mode";
        case ESP_RST_EXT:
            return "Reset by external pin (not applicable for ESP32)";
        case ESP_RST_INT_WDT:
            return "Reset (software or hardware) due to interrupt watchdog";
        case ESP_RST_PANIC:
            return "Software reset due to exception/panic";
        case ESP_RST_POWERON:
            return "Reset due to power-on event";
        case ESP_RST_SDIO:
            return "Reset over SDIO";
        case ESP_RST_SW:
            return "Software reset via esp_restart";
        case ESP_RST_TASK_WDT:
            return "Reset due to task watchdog";
        case ESP_RST_WDT:
            return "ESP_RST_WDT";

        case ESP_RST_UNKNOWN:
        default:
            return "Unknown";
        }
    };

    esp_chip_info_t info;
    esp_chip_info(&info);

    stream.println(cli_colors::red);
    
    stream.println("                  ╔╗╗╖╓,");
    stream.println("       ,   ╓@╗╗╖,  `╙╝╬╫╫╫K╗,");
    stream.println("     ╔Å` $╫╫╫╫╫╫╫╫╫Φ╦, `╙╬╫╫╫╬w");
    stream.println("    ╬╛   `''╙╜╝╬╫╫╫╫╫╫╫N, `╝╫╫╫╬,");
    stream.println("   ╬M  ╓╗KKN╗╦╓. `╙╝╫╫╫╫╫╬╗  ╢╫╫╫");
    stream.println("  ╟Ñ ╔╬╫╫╫╫╫╫╫╫╫╫Φ╦, `╬╫╫╫╫╬w ╙╬╫╫");
    stream.println("  ╫⌐ ╫╫╫╫Å╙╙╜╬╫╫╫╫╫╫╬w `╬╫╫╫╫N  ╬╫N");
    stream.println("  ╫  ╫╫╫╫╬╗╦╖, `╙╬╫╫╫╫╬, ╙╫╫╫╫╬  ╣Ñ");
    stream.println("  ╬H `╬╫╫╫╫╫╫╫╫N, `╬╫╫╫╫¼ `╫╫╫╫Ñ");
    stream.println("  ╘╬    `╙╜╝╫╫╫╫╫Ñ  ╟╫╫╫╫U ╙╫╫╫╫");
    stream.println("   ╚N    ,╓, ╙╫╫╫╫╫  ╬╫╫╫╫  ╬╫╫╫╬");
    stream.println("    ╙╬  ╬╫╫╫N ╙╫╫╫╫N j╫╫╫╫H ╟╫╫╜");
    stream.println("      ╝N`╙╩╜  ,╫╫╫╫Ñ j╫╫╫╫H    ,");
    stream.println("       `╚Nw   ╙╬╫╫╫  ╨╝╬Ñ╜  ,╔Ñ╜");
    stream.println("          `╙╝N╦╓,,    ,,╓╗Φ╝^");
    stream.println("               ``''╙'''`");

    stream.println(cli_colors::reset);


    stream.printf("ESP-IDF Version: %s\n", ESP.getSdkVersion());

    stream.println();
    
    stream.printf("EFuse MAC: %s\n", mac2String(ESP.getEfuseMac()).c_str());
    stream.printf("CPU temperature: %.01f °C\n", temperatureRead());

    stream.printf("\n");
    stream.printf("Chip info:\n");
    stream.printf("  Model: %s\n", ESP.getChipModel());
    stream.printf("  Revison number: %d\n", ESP.getChipRevision());
    stream.printf("  Cores: %d\n", ESP.getChipCores());
    stream.printf("  Clock: %d MHz\n", ESP.getCpuFreqMHz());
    stream.printf("  Features:%s%s%s%s%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? " 802.11bgn " : "",
           info.features & CHIP_FEATURE_BLE ? " BLE " : "",
           info.features & CHIP_FEATURE_BT ? " BT " : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? " Embedded-Flash " : " External-Flash ",
           info.features & CHIP_FEATURE_EMB_PSRAM ? " Embedded-PSRAM" : "");
    
    stream.println();

    stream.printf("Flash size: %d MB (mode: %s, speed: %d MHz)\n", ESP.getFlashChipSize() / (1024 * 1024), getFlashModeStr(), ESP.getFlashChipSpeed() / (1024 * 1024));
    stream.printf("PSRAM size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
    
    stream.println();

    stream.printf("Sketch:\n");
    stream.printf("  Size:         %d KB\n", ESP.getSketchSize() / (1024));
    stream.printf("  MD5:          %s\n", ESP.getSketchMD5().c_str());
#ifndef CONFIG_APP_REPRODUCIBLE_BUILD
    stream.printf("  Compile time: " __DATE__ " " __TIME__ "\n");
#endif

    stream.printf("\nReset reason: %s\n", getResetReasonStr());

    stream.println();
});

*/