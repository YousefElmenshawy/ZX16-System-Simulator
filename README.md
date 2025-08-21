# 🚀 ZX16 System Simulator & Interactive Development Environment (IDE)

**A Complete Educational Computer Architecture Platform**  
**CSCE 2303 – Computer Organization and Assembly Language Programming**  
**Live Demo**: [https://codez16.up.railway.app/](https://codez16.up.railway.app/)

---

## 📘 Overview

The ZX16 System Simulator is a comprehensive educational platform that brings computer architecture concepts to life through an interactive simulation environment. This project implements a complete instruction set simulator (ISS) for the ZX16 RISC architecture, featuring both a powerful desktop application and a modern web-based IDE.

### 🌟 Key Features

- **🌐 Web-based IDE**: Modern, VS Code-inspired interface accessible from any browser
- **🎮 Interactive Graphics**: Real-time 2D tile-based graphics with SFML backend
- **🔊 Audio System**: Complete audio support with sound effects and volume control
- **📋 Comprehensive ISA**: Full ZX16 instruction set with all addressing modes
- **🛠️ Complete Toolchain**: Integrated assembler, simulator, and debugger
- **📱 Multi-Platform**: Desktop C++ application + web interface
- **🎯 Educational Focus**: Designed specifically for computer architecture education

---

## 🌐 Web IDE Features

### **Try it Live**: [https://codez16.up.railway.app/](https://codez16.up.railway.app/)

The web IDE provides a complete development environment for ZX16 assembly programming:

#### 🖥️ Modern Interface
- **VS Code-inspired Design**: Familiar, professional interface
- **Syntax Highlighting**: Full ZX16 assembly syntax highlighting
- **Multi-tab Editor**: Work with multiple files simultaneously
- **Dark/Light Themes**: Customizable appearance
- **Line Numbers**: Professional code editing experience

#### 🔧 Development Tools
- **Real-time Assembly**: Instant compilation and error reporting
- **Step-by-step Debugging**: Execute instructions one at a time
- **Register Monitoring**: Live view of all CPU registers
- **Memory Inspector**: Real-time memory visualization
- **Instruction Tracing**: Complete execution history

#### 🎮 Execution Modes
- **Run Mode**: Execute programs at full speed
- **Step Mode**: Single-step debugging with state inspection
- **Graphics Mode**: Interactive execution with real-time graphics rendering
- **ECALL Support**: Full system call implementation with interactive I/O

#### 📊 Real-time Monitoring
- **Live Register Updates**: See register changes as they happen
- **Memory Visualization**: Hex dump with change highlighting
- **Execution Log**: Complete instruction trace
- **Performance Metrics**: Execution statistics and timing

---

## 🏗️ Architecture Overview

### 💾 ZX16 ISA Implementation

The simulator implements the complete ZX16 instruction set architecture:

#### **Instruction Formats**
- **R-Type**: Register-to-register operations
- **I-Type**: Immediate operations and loads
- **S-Type**: Store operations
- **B-Type**: Branch and conditional operations
- **J-Type**: Jump and subroutine calls
- **U-Type**: Upper immediate operations
- **L-Type**: Load operations with addressing modes
- **SYS-Type**: System calls and privileged operations

#### **Complete Instruction Set**
```assembly
# Arithmetic & Logic
ADD, SUB, SLT, SLTU, SLL, SRL, SRA, OR, AND, XOR
ADDI, SLTI, SLTUI, ORI, ANDI, XORI, SLLI, SRLI, SRAI

# Memory Operations  
LB, LW, LBU, SB, SW

# Control Flow
BEQ, BNE, BZ, BNZ, BLT, BGE, BLTU, BGEU
J, JAL, JR, JALR

# Upper Immediate
LUI, AUIPC

# System Calls
ECALL (with 10 service numbers)

# Pseudo-instructions
LI, LI16, LA, PUSH, POP, CALL, RET, INC, DEC, NEG, NOT, CLR, NOP
```

### 🎮 Graphics & Multimedia System

#### **2D Tile-Based Graphics**
- **Resolution**: 320×240 pixels (QVGA)
- **Tile System**: 16×16 pixel tiles (20×15 grid)
- **Color Depth**: 16-color palette (4-bit pixels)
- **Memory-Mapped**: Direct memory access to graphics buffers
- **Automatic Rendering**: Graphics are rendered automatically without manual calls

#### **Memory Layout**
| Component | Address Range | Description |
|-----------|---------------|-------------|
| Tile Map Buffer | `0xF000 - 0xF12B` | Screen tile layout |
| Tile Definitions | `0xF200 - 0xFA00` | 16×16 pixel tile data |
| Color Palette | `0xFA00 - 0xFA0F` | 16 RGB color entries |

#### **Audio System**
- **Sound Effects**: WAV file playbook with frequency control
- **Volume Control**: Programmable audio levels
- **Real-time Playback**: Synchronized with program execution

---

## 🛠️ System Calls (ECALL Services)

The ZX16 simulator provides comprehensive system call support:

| Service | Function | Description |
|---------|----------|-------------|
| `ecall 1` | String Input | Read string from user into memory buffer |
| `ecall 2` | Integer Input | Read integer from user |
| `ecall 3` | String Output | Print null-terminated string from memory |
| `ecall 4` | Audio Playback | Play sound effect with frequency control |
| `ecall 5` | Volume Control | Set master audio volume (0-255) |
| `ecall 6` | Stop Audio | Stop all currently playing audio |
| `ecall 7` | Keyboard Input | Read single key press |
| `ecall 8` | Register Dump | Display all register values |
| `ecall 9` | Memory Dump | Display memory region in hex |
| `ecall 10` | Program Exit | Terminate program execution |

---

## 🎯 Featured Demo: Interactive Pong Game

The simulator includes a complete, playable Pong game that demonstrates advanced features:

### **Game Features**
- **Real-time Graphics**: Smooth paddle and ball movement
- **Interactive Controls**: Keyboard input for paddle control
- **Sound Effects**: Audio feedback for ball hits and scoring
- **Score Display**: Live score tracking with custom tile graphics
- **Game Logic**: Complete collision detection and physics

### **Technical Highlights**
- **Multi-mode Execution**: Runs in both desktop and web environments
- **Memory-Mapped I/O**: Direct graphics memory manipulation
- **Audio Integration**: Synchronized sound effects
- **Input Handling**: Real-time keyboard processing
- **State Management**: Complex game state tracking

---

## 🚀 Getting Started

### **Option 1: Web IDE (Recommended)**
Simply visit [https://codez16.up.railway.app/](https://codez16.up.railway.app/) and start coding immediately!

### **Option 2: Local Development**

#### **Prerequisites**
- **C++ Compiler**: GCC 7.3+ or equivalent
- **SFML**: Simple and Fast Multimedia Library
- **Python 3.11+**: For the assembler and web backend
- **FastAPI**: For local web server (optional)

#### **Build Instructions**

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/your-username/zx16-system-simulator.git
   cd zx16-system-simulator
   ```

2. **Build the Simulator**:
   ```bash
   cd Src
   g++ -std=c++17 -o ZX16_System_Simulator *.cpp -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
   ```

3. **Install Python Dependencies**:
   ```bash
   pip install fastapi uvicorn
   ```

#### **Running Programs**

**Desktop Mode**:
```bash
# Assemble your program
python app/Backend/zx16asm.py program.s -o program.bin

# Run in different modes
./ZX16_System_Simulator program.bin                # Normal execution
./ZX16_System_Simulator program.bin step          # Step-by-step debugging  
./ZX16_System_Simulator program.bin interactive   # Graphics mode
```

**Web Mode**:
```bash
uvicorn app.Backend.main:app --reload
# Open http://localhost:8000
```

---

## 📚 Programming Examples

### **Basic Assembly Program**
```assembly
# Simple arithmetic example
li s0, 10          # Load immediate value 10 into s0
li s1, 5           # Load immediate value 5 into s1
add s0, s1         # Add s1 to s0 (s0 = 15)
ecall 8            # Dump registers to see result
ecall 10           # Exit program
```

### **Graphics Programming**
```assembly
# Set up tile map for simple graphics
li16 a0, 0xF000    # Tile map buffer address
li t0, 1           # Tile number (ball)
sb t0, 150(a0)     # Place tile at center of screen
# Graphics render automatically - no ecall needed!
ecall 10           # Exit
```

### **Interactive Input/Output**
```assembly
.data
prompt: .string "Enter your name: "
buffer: .space 50

.text
li16 a0, prompt    # Load prompt address
ecall 3            # Print prompt
li16 a0, buffer    # Load buffer address  
li a1, 50          # Buffer size
ecall 1            # Read string
li16 a0, buffer    # Load buffer address
ecall 3            # Print the string back
ecall 10           # Exit
```

---

## 🧪 Test Suite & Validation

The project includes comprehensive test cases covering all aspects of the ZX16 architecture:

### **Core Test Categories**

| Test Case | Description | Features Tested | Expected Behavior |
|-----------|-------------|-----------------|-------------------|
| **TC-ZX16-01** | Basic Instruction Execution | • LI (Load Immediate)<br>• ADD (Addition)<br>• BLT (Branch Less Than)<br>• Basic branching logic | Tests fundamental arithmetic and conditional branching with simple register operations |
| **TC-ZX16-02** | Upper Immediate & Shift Operations | • LUI (Load Upper Immediate)<br>• SLLI/SRLI (Shift Left/Right Logical)<br>• ECALL 8 (Register Dump)<br>• ECALL 10 (Program Exit) | Validates upper immediate loading and bit shifting operations with system calls |
| **TC-ZX16-03** | Loop & Branch Instructions | • Loop implementation<br>• MV (Move/Copy)<br>• ADDI (Add Immediate)<br>• BNE (Branch Not Equal)<br>• Counter-based iteration | Tests loop structures, counters, and conditional branching for iterative algorithms |
| **TC-ZX16-04** | String Processing & Character Analysis | • String data section<br>• LB (Load Byte)<br>• Character comparison<br>• String traversal<br>• Null terminator detection | Validates memory access, string processing, and character-level operations |
| **TC-ZX16-05** | ECALL Services & I/O Operations | • ECALL 2 (Integer Input)<br>• ECALL 3 (String Output)<br>• LA (Load Address)<br>• Interactive user input/output | Tests system call interface for user interaction and I/O operations |
| **TC-ZX16-06** | ECALL Integration Testing | • Multiple ECALL services<br>• Data flow between calls<br>• Error handling<br>• Service chaining | Comprehensive testing of system call integration and data handling |
| **TC-ZX16-07** | Memory Operations & Bubble Sort | • SW/LW (Store/Load Word)<br>• SB/LB (Store/Load Byte)<br>• Array manipulation<br>• Bubble sort algorithm<br>• Memory addressing | Tests memory operations and complex algorithms requiring array access |
| **TC-ZX16-08** | Graphics & Tile System | • Memory-mapped graphics<br>• Tile map buffer (0xF000)<br>• Tile definitions<br>• 2D graphics rendering<br>• Color palette usage | Validates graphics memory layout and tile-based rendering system |
| **TC-ZX16-09** | Jump & Link Instructions | • JAL (Jump and Link)<br>• JR (Jump Register)<br>• Function calls<br>• Return addresses<br>• Stack operations | Tests subroutine calls, function linkage, and return address management |
| **TC-ZX16-10** | Audio System Functionality | • ECALL 4 (Audio Playback)<br>• ECALL 5 (Volume Control)<br>• ECALL 6 (Stop Audio)<br>• Sound frequency control<br>• Audio buffer management | Validates audio system integration and sound effect playback |

### **Test Files Structure**
```
TestCases/
├── TC-ZX16-01.s    # Source assembly code
├── TC-ZX16-01.bin  # Assembled binary
├── TC-ZX16-02.s    # Upper immediate & shifts
├── TC-ZX16-02.bin  
├── ...
├── TC-ZX16-10.s    # Audio system test
├── Execution Tests.cpp       # C++ test harness
└── Disassembly Tests.cpp    # Disassembly validation
```

### **Validation Features**
- **Automated Testing**: Comprehensive test suite with expected outputs
- **Register Validation**: Automatic register state verification
- **Memory Checking**: Memory content validation
- **Instruction Tracing**: Complete execution path verification

---

## 🔧 Advanced Features

### **Professional Assembler**
- **Two-pass Assembly**: Complete symbol resolution
- **Multiple Output Formats**: Binary, Intel HEX, Verilog, Memory files
- **Error Reporting**: Detailed syntax and semantic error messages
- **Symbol Tables**: Complete symbol management and resolution
- **Pseudo-instruction Support**: High-level programming constructs
- **External Assembler Support**: Compatible with the [ZX16 assembler](https://github.com/shalan/zx16.git) for advanced assembly features and additional toolchain integration

### **Debugging & Analysis**
- **Step-by-step Execution**: Single instruction debugging
- **Breakpoint Support**: Set execution breakpoints
- **State Inspection**: Real-time register and memory monitoring
- **Execution Profiling**: Performance analysis and statistics
- **Memory Visualization**: Graphical memory layout display

### **Graphics Engine**
- **SFML Integration**: Professional graphics library backend
- **Real-time Rendering**: 60 FPS graphics updates
- **Memory-Mapped Graphics**: Direct memory access to graphics buffers
- **Custom Tile System**: User-definable graphics tiles
- **Color Management**: Flexible palette system

---

## 🌐 Deployment & Infrastructure

### **Cloud Deployment**
- **Platform**: Railway.app hosting
- **Architecture**: FastAPI backend + SFML graphics
- **Containerization**: Docker-based deployment
- **Virtual Display**: Xvfb for headless graphics
- **Auto-scaling**: Cloud-native scalability

### **Technology Stack**
- **Backend**: FastAPI (Python) + C++ simulator
- **Frontend**: Modern HTML5/CSS3/JavaScript
- **Graphics**: SFML (Simple and Fast Multimedia Library)
- **Build System**: CMake + Docker
- **Deployment**: Railway + Docker containers

---

## 🎓 Educational Applications

### **Course Integration**
- **Computer Architecture**: ISA design and implementation
- **Assembly Programming**: Low-level programming concepts
- **Operating Systems**: System call interfaces
- **Computer Graphics**: 2D graphics programming


### **Learning Outcomes**
- Understanding of RISC architecture principles
- Assembly language programming proficiency
- System-level programming concepts
- Graphics and multimedia programming
- Debugging and performance analysis skills

---

## 🛣️ Future Enhancements

### **Planned Features**
- **Pipeline Visualization**: Show instruction pipeline stages
- **Cache Simulation**: Memory hierarchy simulation
- **Network Programming**: Socket-based communication
- **Advanced Graphics**: 3D rendering capabilities
- **Virtual Machine**: Complete OS simulation environment

### **Community Contributions**
We welcome contributions in the following areas:
- Additional test cases and examples
- Documentation improvements
- Performance optimizations
- New instruction set extensions
- Educational materials and tutorials

---

## 📖 Documentation & Resources

### **Architecture Reference**
- **ZX16 ISA Manual**: Complete instruction set documentation
- **Programming Guide**: Assembly language programming tutorial
- **System Call Reference**: Detailed ECALL documentation
- **Graphics Programming**: Tile-based graphics tutorial

### **Development Resources**
- **API Documentation**: Complete simulator API reference
- **Build Instructions**: Detailed setup and compilation guide
- **Deployment Guide**: Cloud deployment instructions
- **Contributing Guidelines**: Development workflow and standards

---

## 🏆 Project Achievements

### **Technical Milestones**
- ✅ Complete ZX16 ISA implementation
- ✅ Professional web-based IDE
- ✅ Real-time graphics and audio
- ✅ Cloud deployment and scalability
- ✅ Comprehensive test suite
- ✅ Interactive demo applications

### **Educational Impact**
- **Hands-on Learning**: Interactive computer architecture education
- **Real-world Skills**: Professional development environment experience
- **Project-based Learning**: Complete system implementation
- **Industry Relevance**: Modern toolchain and deployment practices

---

## 👥 Development Team

- Yousef Elmenshawy
- Kareem Rashed
- Doha Deia
- Habiba Saad

---

## 📄 License & Attribution

**License**: MIT License  
**Institution**: American University of Cairo  
**Course**: CSCE 2303 – Computer Organization and Assembly Language Programming

### **Acknowledgments**
- **SFML Library**: Graphics and multimedia framework
- **FastAPI**: Modern web framework
- **Railway.app**: Cloud hosting platform
- **Open Source Community**: Various libraries and tools

---

## 🔗 Quick Links

- **🌐 Live Demo**: [https://codez16.up.railway.app/](https://codez16.up.railway.app/)
- **📚 Documentation**: [Project Wiki](https://github.com/YousefElmenshawy/zx16-system-simulator/wiki)
- **🐛 Bug Reports**: [Issues Page](https://github.com/YousefElmenshawy/zx16-system-simulator/issues)


---

## 🚀 Get Started Now!

**Ready to explore computer architecture?**

1. **🌐 Try the Web IDE**: Visit [https://codez16.up.railway.app/](https://codez16.up.railway.app/)
2. **📖 Read the Tutorial**: Start with basic assembly programming
3. **🎮 Play the Demo**: Try the interactive Pong game
4. **🔧 Build Your Own**: Create custom ZX16 programs
5. **🤝 Join the Community**: Contribute to the project

**The future of computer architecture education is here!** 🎓✨
