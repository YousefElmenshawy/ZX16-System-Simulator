# 🚀 ZX16 Instruction Set Simulator (ISS)

**CSCE 2303 – Computer Organization and Assembly Language Programming**
**Summer 2025**
**Project 1: ZX16 ISS**

---

## 📘 Overview

This project implements a **simulator for the ZX16 Instruction Set Architecture (ISA)**, an open-source RISC-inspired ISA developed at AUC for educational purposes. The simulator reads a binary machine code file, decodes and executes each instruction, and replicates the behavior of a ZX16 CPU. It includes support for system calls (ecalls), 2D tile-based graphics, and basic audio output.

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

> Ensure that your `.bin` file is a valid ZX16 binary produced by the official assembler from the [ZX16 GitHub repo](https://github.com/shalan/ZX16).


## 📦 Usage Guidelines

### 📌 Command Syntax:

```bash
zx16sim <machine_code_file_name>
```

### ✅ Supported Features:

* Full decoding and execution of all ZX16 instructions.
* Instruction tracing: prints human-readable form of each instruction.
* Register and memory state updates.
* System call (ecall) handling:

  * String and integer I/O
  * Audio control
  * 2D graphics rendering
  * Memory and register dumps
* 2D Tiled graphics support (QVGA: 320x240 screen).
* Logging of execution flow for debugging.


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


## 🧱 Project Challenges

// will be added when finishing the project 

## ❌ Known Limitations

// will be added when finishing the project 


## 📚 References

* ZX16 ISA GitHub: [https://github.com/shalan/ZX16](https://github.com/shalan/ZX16)
* Command-line arguments in C: [Tutorialspoint](https://www.tutorialspoint.com/cprogramming/c_command_line_arguments.htm)
