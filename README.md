# GGBoy-Core  
**A Lightweight C++ GameBoy and GameBoy Color Emulator**  

GGBoy-Core is an C++ emulator project focused on exploring the architecture of Nintendo's classic GameBoy (**DMG**) and GameBoy Color (**GBC**). While not aiming for cycle-accurate precision, it provides functional emulation for both systems, included features:  
- **DMG mode** for original GameBoy games  
- **GBC compatibility**
- **audio emulation** implementing the GameBoy's APU (Audio Processing Unit)
- **Savestates**
- **Emulation Speedup**

Designed as a dependency-free core, it serves as a foundation to build custom frontends or experiment with GameBoy hardware emulation. The project requires only a **C++17-compliant compiler** and avoids external libraries.


## Development Resources  
The following resources were instrumental in understanding GameBoy hardware:  
- [gbdev's Awesome GBDev](https://github.com/gbdev/awesome-gbdev#emulator-development)  
- [Pan Docs](https://gbdev.io/pandocs/)  
- [GBDev Wiki](https://gbdev.gg8.se/wiki/articles/Main_Page)  
- [GameBoy OPcodes](https://izik1.github.io/gbops/)  
- [RGBDS Documentation](https://rgbds.gbdev.io/docs/master/gbz80.7)  
- [GB Test ROMs](https://github.com/retrio/gb-test-roms/tree/master)  
- [MagenTests](https://github.com/alloncm/MagenTests)  