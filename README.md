# 🚀 ZX16 Instruction Set Simulator (ISS)

**CSCE 2303 – Computer Organization and Assembly Language Programming**
**Summer 2025**
**Project 1: ZX16 ISS**

---

## 📘 Overview

This project implements a **simulator for the ZX16 Instruction Set Architecture (ISA)**, an open-source RISC-inspired ISA developed at AUC for educational purposes. The simulator reads a binary machine code file, decodes and executes each instruction, and replicates the behavior of a ZX16 CPU. It includes support for system calls (ecalls), 2D tile-based graphics, and basic audio output. The project culminates in a fully functional, interactive **Pong-style game**, which serves as a comprehensive demonstration of the simulator's capabilities.

## 🛠️ Build Instructions

> **Language**: C++

### 🔧 To Build (C++):

1. Clone the repository:

   git clone https://github.com/your-username/zx16-sim.git
   cd zx16-sim

2. Compile:

   g++ -std=c++17 -o zx16sim src/main.cpp

3. Run the simulator:

   ./zx16sim program.bin

> Ensure that your `.bin` file is a valid ZX16 binary produced by the official assembler from the [ZX16 GitHub repo](https://github.com/shalan/z16.git).

4. 🎮 Running the Pong Game:

To run the included Pong game, ensure that `main.cpp` is configured to load the correct binary file (`game.bin`)

> sim.loadBinaryFile("../game.bin") in the main.cpp file.
> Then, compile and run the simulator to start the game



## 📦 Usage Guidelines

### 📌 Command Syntax:

```bash
zx16sim <machine_code_file_name>
```

### ✅ Supported Features:

*   **Full ISA Decoding**: Decodes and executes all ZX16 instructions, including R-Type, I-Type, S-Type, B-Type, L-Type, J-Type, and U-Type.
*   **Instruction Tracing**: Prints a human-readable, disassembled form of each instruction as it is executed.
*   **System Call (Ecall) Handling**: Provides a comprehensive set of services to interact with the system:
    *   `ecall 1`: Read a string from the user into a memory buffer.
    *   `ecall 2`: Read an integer from the user.
    *   `ecall 3`: Print a null-terminated string from memory.
    *   `ecall 4`: Play a predefined sound effect (e.g., ball hit, lose sound).
    *   `ecall 5`: Set the master audio volume.
    *   `ecall 6`: Stop all currently playing audio.
    *   `ecall 7`: Read a single key press from the keyboard.
    *   `ecall 8`: Dump the current state of all registers to the console.
    *   `ecall 9`: Dump a specified region of memory to the console.
    *   `ecall 10`: Halt the program.
    *   `ecall 11`: Render the graphics frame to the screen.
*   **2D Tiled Graphics** (QVGA: 320x240 screen): Supports a complete graphics pipeline with a 320x240 screen, custom tile definitions, and a 16-color palette, all managed through memory-mapped I/O.
*   **Audio Playback**: Capable of playing WAV sound files for audio feedback in applications like the Pong game
*   **Memory and register dumps**: Provides debugging system calls to inspect the machine's state. `ecall 8` prints the current values of all eight general-purpose registers, while `ecall 9` prints a hex dump of a specified memory region. These are crucial for verifying the correctness of algorithms.
*   **Logging of execution flow for debugging**: The simulator prints a dynamic, human-readable trace of every instruction as it is executed. Each line of the log shows the memory address (PC), the 16-bit instruction in hexadecimal, and its disassembled form, providing a clear, step-by-step view of the program's execution path.


## 🧠 Design Overview

### 💡 Architecture:

* **Memory Module**: Simulates system memory, supporting the loading of binary code and managing memory-mapped graphics and audio.
* **Register File**: 32 general-purpose registers.
* **Instruction Decoder**: Parses 16-bit or 32-bit instruction words and identifies the instruction type and operands.
* **Execution Engine**: Executes the decoded instruction and updates CPU state.
* **Ecall Handler**: Processes system calls via service numbers.
* **Graphics Module**: Implements 2D tile-mapped screen rendering.
* **Audio Module**: Simulates tone playing and audio controls.

### 🧱 Memory Mapping:

| Component        | Address Range     |
| ---------------- | ----------------- |
| Tile Map Buffer  | `0xF000`          |
| Tile Definitions | `0xF200 - 0xFA00` |
| Color Palette    | `0xFA00 - 0xFA0F` |

### 🎨 Graphics System:

* Resolution: 320 × 240 pixels (QVGA)
* Tiles: 16×16 pixels each
* Total: 300 tiles (20x15)
* Palette: 16 RGB colors (4-bit pixel encoding)

---

## 🧪 Testing Guidelines

All test cases are in the `tests/` directory.

### ✅ Test Documentation Format:

| Field               | Description                              |
| ------------------- | ---------------------------------------- |
| **Test Case ID**    | Unique name (e.g., `TC-ZX16-01`)         |
| **Objective**       | Describes feature/instruction tested     |
| **Test Steps**      | How to run it                            |
| **Expected Output** | Registers, memory state, printed strings |

### 🧾 Directory Structure:

tests/
├── TC-ZX16-01.s          # Assembly source
├── TC-ZX16-01.bin        # Binary file
├── TC-ZX16-01.expected   # Expected output
├── TC-ZX16-01.log        # Actual output (optional)

### ✅ Required Test Coverage:

* Arithmetic and logical instructions
* Load/store instructions
* Branching and control flow
* All ecall services (1 to 10)
* Graphics rendering
* Audio functionality
* (Jump Register), error handling

### ✅ Featured Test Case: Pong Game

The primary test case for this simulator is a complete **Pong game** (`testA.s`). This test provides comprehensive coverage of the simulator's most critical features by integrating:
*   **Graphics Rendering (`ecall 11`)**: The game continuously draws the paddles and ball to the tile map and renders them to the screen.
*   **Interactive Input (`ecall 7`)**: It reads keyboard input to move the player paddles in real-time.
*   **Audio Feedback (`ecall 4`)**: It plays sound effects when the ball hits a paddle or when a player scores.

## 🧱 Project Challenges

*   **Limited Instruction Offsets**: The small immediate fields for branch and jump instructions (`j`, `beq`, etc.) created significant challenges when writing larger programs like the Pong game. It required careful code organization and manual address calculation to ensure all parts of the assembly code were reachable.
*   **SFML Performance Tuning**: Achieving smooth, consistent movement for the game's ball was difficult. The speed of the simulation loop had to be carefully balanced with SFML's rendering calls to prevent the ball from moving too fast or too slow, which required implementing manual delay loops in assembly.
*   **Audio Synchronization**: There is a noticeable delay before the 'lose' sound effect plays upon game completion. Synchronizing audio playback with the final moments of the simulation before it halts proved to be an issue.
*   **Implementing and packing 4-bit color data** into memory for 2D tiles.
*   **Designing accurate memory-mapped regions** for graphics and audio buffers.

## ❌ Known Limitations

**No Simultaneous Key Presses**: The keyboard input handling (`ecall 7`) can only process one key press at a time. As a result, two players cannot move their paddles simultaneously, making multiplayer gameplay turn-based.
*   **Simplistic Ball Physics**: The ball in the Pong game moves at a constant velocity and reflects off surfaces at a fixed angle. The provided code does not implement advanced physics like spin or variable speed, limiting gameplay depth.
*   **Memory size is capped** to a predefined maximum due to platform simplicity.


## 📚 References

* ZX16 ISA GitHub: [https://github.com/shalan/ZX16](https://github.com/shalan/ZX16)
* Command-line arguments in C: [Tutorialspoint](https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm)
