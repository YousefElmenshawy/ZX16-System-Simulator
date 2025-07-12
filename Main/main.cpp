#include <iostream>
#include<cstdlib>
#include "ZX16_Simulator.h"
#include "Graphics.h"
#include <thread>
using namespace std;

int main() {
    ZX16_Simulator sim;
    sim.loadBinaryFile("../game.bin");

    GraphicsMemory Gmem;
    Gmem.setMemory(sim.getMemoryPtr());

    Graphics gfx;
    gfx.setmemory(&Gmem);

    // Preferred setup: Interactive simulation
    sim.runInteractive(&gfx);  // run the game live
}


