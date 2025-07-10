#include <iostream>
#include<cstdlib>
#include "ZX16_Simulator.h"
#include "Graphics.h"
#include <thread>
using namespace std;

int main() {
    ZX16_Simulator sim;
    sim.loadBinaryFile("/Users/kareem/Desktop/Projects/ZX16-System-Simulator/Main/Game.bin");

    GraphicsMemory Gmem;
    Gmem.setMemory(sim.getMemoryPtr());

    Graphics gfx;
    gfx.setmemory(&Gmem);

    // Preferred setup: Interactive simulation
    sim.runInteractive(&gfx);  // run the game live
}


