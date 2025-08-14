from fastapi import FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
import subprocess
import uuid
import os
from fastapi.responses import JSONResponse
import time
import re

app = FastAPI()

# Add CORS middleware configuration
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Allow frontend origin
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Store the latest binary file for step mode
latest_step_bin = None
step_count = 0
step_proc = None
step_output_buffer = ""
prev_registers = {}  # Store previous register values to detect changes
prev_memory = []  # Store previous memory for run/step mode

@app.get("/state")
def get_default_state():
    registers = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
    memory = []
    for addr in range(0, 65536, 16):
        memory.append({
            "address": f"0x{addr:04X}",
            "bytes": ["00"] * 16
        })
    global prev_memory
    prev_memory = memory.copy()
    return {"output": "Ready to run...", "registers": registers, "memory": memory}

@app.post("/simulate")
async def simulate_code(request: Request):
    global prev_memory, prev_registers
    try:
        data = await request.json()
        code = data["code"]

        uid = str(uuid.uuid4())
        asm_file = f"temp_{uid}.s"
        bin_file = f"temp_{uid}.bin"

        with open(asm_file, "w") as f:
            f.write(code)

        current_dir = os.path.dirname(os.path.abspath(__file__))
        assemble_cmd = ["python", os.path.join(current_dir, "zx16asm.py"), asm_file, "-o", bin_file]
        try:
            result = subprocess.run(assemble_cmd, capture_output=True, text=True, cwd=current_dir, timeout=30)
        except subprocess.TimeoutExpired:
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return {"output": "Assembly process timed out!"}

        output = ""
        if result.returncode != 0:
            output = f"Assembly failed.\n{filter_output(result.stderr)}\n{filter_output(result.stdout)}"
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return {"output": output, "registers": None, "memory": None}

        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        sim_cmd = [simulator_path, bin_file]
        try:
            result = subprocess.run(sim_cmd, capture_output=True, text=True, cwd=current_dir, timeout=30)
        except subprocess.TimeoutExpired:
            if os.path.exists(asm_file):
                os.remove(asm_file)
            if os.path.exists(bin_file):
                os.remove(bin_file)
            return {"output": "Simulation process timed out!"}

        # Clean up temp files
        if os.path.exists(asm_file):
            os.remove(asm_file)
        if os.path.exists(bin_file):
            os.remove(bin_file)

        if result.returncode != 0:
            output = f"Simulation failed.\n{filter_output(result.stderr)}\n{filter_output(result.stdout)}"
            return {"output": output, "registers": None, "memory": None}
        else:
            output = filter_output(result.stdout)
            if result.stderr:
                stderr_filtered = filter_output(result.stderr)
                if stderr_filtered:
                    output += f"\n{stderr_filtered}"

        default_registers = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
        default_memory = []
        for addr in range(0, 65536, 16):
            default_memory.append({
                "address": f"0x{addr:04X}",
                "bytes": ["00"] * 16
            })
        registers = default_registers.copy()
        memory = [dict(m) for m in default_memory]
        lines = result.stdout.splitlines()
        reg_section = False
        mem_section = False
        mem_start = 0
        mem_size = 65536
        for line in lines:
            line_strip = line.strip()
            for reg in registers.keys():
                if line_strip.startswith(reg + " ="):
                    reg_val = line_strip.split('=')[1].strip()
                    try:
                        val = int(reg_val)
                        # Convert to signed 16-bit for display
                        if val >= 2**15:
                            val -= 2**16
                        registers[reg] = val
                        # Also replace the value in the output string for correct sign display
                        output = output.replace(f"{reg} = {reg_val}", f"{reg} = {val}")
                    except ValueError:
                        pass
                    break
            # --- Memory parsing ---
            if line_strip.startswith("Memory Dump"):
                import re
                m = re.search(r"address\s*=\s*(\d+),\s*size\s*=\s*(\d+)", line_strip)
                if m:
                    mem_start = int(m.group(1))
                    mem_size = int(m.group(2))
                mem_section = True
                continue
            if mem_section:
                if line_strip == '':
                    mem_section = False
                    continue
                if ':' in line_strip:
                    addr, bytestr = line_strip.split(':', 1)
                    addr_int = int(addr.strip(), 16)
                    bytes_list = bytestr.strip().split()
                    idx = addr_int // 16
                    if 0 <= idx < len(memory):
                        memory[idx]["bytes"] = bytes_list
            elif line_strip.startswith("Memory Dump"):
                mem_section = True

        prev_memory = [dict(m) for m in memory]
        prev_registers = registers.copy()  # <-- Update prev_registers here

        # Identify changed registers (for highlighting, optional)

        # (No previous state to compare in run mode, so leave empty or compare to default_registers if desired)

        # Identify changed registers (for highlighting, optional)
        changed_registers = []
        # (No previous state to compare in run mode, so leave empty or compare to default_registers if desired)

        return {"output": output, "registers": registers, "memory": prev_memory, "changed_registers": changed_registers}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

@app.post("/simulate_step")
async def simulate_step(request: Request):
    global latest_step_bin, step_proc, step_output_buffer, step_count, prev_registers, prev_memory
    try:
        data = await request.json()
        code = data["code"]

        uid = str(uuid.uuid4())
        asm_file = f"temp_{uid}.s"
        bin_file = f"temp_{uid}.bin"

        with open(asm_file, "w") as f:
            f.write(code)

        current_dir = os.path.dirname(os.path.abspath(__file__))
        assemble_cmd = ["python", os.path.join(current_dir, "zx16asm.py"), asm_file, "-o", bin_file]
        try:
            result = subprocess.run(assemble_cmd, capture_output=True, text=True, cwd=current_dir, timeout=30)
        except subprocess.TimeoutExpired:
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return {"output": "Assembly process timed out!"}

        if os.path.exists(asm_file):
            os.remove(asm_file)

        if result.returncode != 0:
            output = f"Assembly failed.\nStderr:\n{result.stderr}\nStdout:\n{result.stdout}"
            if os.path.exists(bin_file):
                os.remove(bin_file)
            return {"output": output, "registers": None, "memory": None}

        # Terminate previous process if running
        if step_proc and step_proc.poll() is None:
            step_proc.terminate()
        step_proc = None
        step_output_buffer = ""

        # Start persistent simulator process in step mode
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        step_proc = subprocess.Popen(
            [simulator_path, bin_file, "step"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=current_dir,
            text=True,
            bufsize=1
        )
        latest_step_bin = bin_file
        step_count = 0  # Reset step count on new code
        prev_registers = {}  # Reset previous registers
        prev_memory = []  # Reset previous memory

        return {"output": "Assembly succeeded. Ready for step mode.", "registers": None, "memory": None}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

@app.post("/step")
async def step_simulation():
    global latest_step_bin, step_count, prev_registers, prev_memory, step_proc
    try:
        if not step_proc or step_proc.poll() is not None:
            return {"output": "Step mode not active or process ended.", "registers": None, "memory": None}

        step_proc.stdin.write("\n")
        step_proc.stdin.flush()

        output = ""
        timeout = 5.0
        start_time = time.time()
        registers_found = False
        memory_dump_started = False
        memory_dump_ended = False
        current_instruction = None
        current_pc = None

        while True:
            try:
                line = step_proc.stdout.readline()
                if not line:
                    break
                output += line

                # Track what sections we've seen and capture important info
                if "Current PC:" in line:
                    registers_found = True
                    pc_match = re.search(r"0x([0-9a-fA-F]+)", line)
                    if pc_match:
                        current_pc = pc_match.group(1)
                elif "Executed:" in line:
                    inst_match = re.search(r"Executed:\s*(.+)", line)
                    if inst_match:
                        current_instruction = inst_match.group(1).strip()
                elif "Memory Dump" in line:
                    memory_dump_started = True
                elif memory_dump_started and line.strip() == "":
                    memory_dump_ended = True

                if "Simulation ended." in line or "PC out of program bounds" in line:
                    for _ in range(4100):
                        try:
                            extra_line = step_proc.stdout.readline()
                            if not extra_line:
                                break
                            output += extra_line
                        except:
                            break
                    break

                if registers_found and memory_dump_ended:
                    break

                if time.time() - start_time > timeout:
                    break
            except:
                break

        # Filter the output to show only important lines
        filtered_output = filter_output(output)

        simulation_ended = "Simulation ended." in output or "PC out of program bounds" in output
        if simulation_ended:
            step_proc.terminate()
            step_proc = None
            step_count = 0

        # Parse registers from the output
        registers = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
        lines = output.splitlines()
        for line in lines:
            line_strip = line.strip()
            for reg in registers.keys():
                if line_strip.startswith(reg + " ="):
                    reg_val = line_strip.split('=')[1].strip()
                    try:
                        val = int(reg_val)
                        if val >= 2**15:
                            val -= 2**16
                        registers[reg] = val
                    except ValueError:
                        pass
                    break

        # Parse memory from the output (similar to simulate function)
        default_memory = []
        for addr in range(0, 65536, 16):
            default_memory.append({
                "address": f"0x{addr:04X}",
                "bytes": ["00"] * 16
            })
        memory = [dict(m) for m in default_memory]

        mem_section = False
        for line in lines:
            line_strip = line.strip()
            # --- Memory parsing ---
            if line_strip.startswith("Memory Dump"):
                mem_section = True
                continue
            if mem_section:
                if line_strip == '':
                    mem_section = False
                    continue
                if ':' in line_strip:
                    addr, bytestr = line_strip.split(':', 1)
                    addr_int = int(addr.strip(), 16)
                    bytes_list = bytestr.strip().split()
                    idx = addr_int // 16
                    if 0 <= idx < len(memory):
                        memory[idx]["bytes"] = bytes_list

        # Identify changed registers
        changed_registers = []
        if prev_registers:
            for reg, val in registers.items():
                if reg in prev_registers and prev_registers[reg] != val:
                    changed_registers.append(reg)
        prev_registers = registers.copy()

        # Identify changed memory addresses
        changed_memory = []
        if prev_memory:
            for i, mem_row in enumerate(memory):
                if i < len(prev_memory):
                    prev_bytes = prev_memory[i]["bytes"]
                    curr_bytes = mem_row["bytes"]
                    if prev_bytes != curr_bytes:
                        # Find which specific bytes changed
                        changed_bytes = []
                        for j, (prev_byte, curr_byte) in enumerate(zip(prev_bytes, curr_bytes)):
                            if prev_byte != curr_byte:
                                changed_bytes.append(j)
                        if changed_bytes:
                            changed_memory.append({
                                "row_index": i,
                                "address": mem_row["address"],
                                "changed_bytes": changed_bytes
                            })
        prev_memory = [dict(m) for m in memory]

        # Extract current PC and instruction for highlighting
        for line in lines:
            if "Current PC:" in line:
                pc_match = re.search(r"0x([0-9a-fA-F]+)", line)
                if pc_match:
                    current_pc = pc_match.group(1)
            if "Executed:" in line:
                inst_match = re.search(r"Executed:\s*(.+)", line)
                if inst_match:
                    current_instruction = inst_match.group(1).strip()

        return {
            "output": filtered_output,  # Return filtered output
            "registers": registers,
            "memory": memory,
            "changed_registers": changed_registers,
            "changed_memory": changed_memory,
            "current_pc": current_pc,
            "current_instruction": current_instruction,
            "simulationEnded": simulation_ended
        }
    except Exception as e:
        if step_proc:
            step_proc.terminate()
            step_proc = None
        return {"output": f"Step error: {str(e)}", "registers": None, "memory": None, "simulationEnded": True}

def filter_output(text):
    if not text:
        return ""
    filtered_lines = []
    for line in text.split('\n'):
        if (
            'simulation' in line.lower() or
            'ECALL' in line):
            filtered_lines.append(line)
    return '\n'.join(filtered_lines)

@app.post("/step")
async def step_code(request: Request):
    global step_proc, latest_step_bin, step_count, prev_registers, prev_memory
    try:
        if not step_proc or step_proc.poll() is not None:
            return {"output": "No active simulation. Please compile and run first.", "registers": None, "memory": None}

        # Send newline to trigger next step
        step_proc.stdin.write('\n')
        step_proc.stdin.flush()

        # Read output until we get a complete step
        output = ""
        while True:
            line = step_proc.stdout.readline()
            if not line:
                break
            output += line
            # Check if we've reached the end of the step
            if "Executed" in line or "ecall" in line.lower():
                break

        # Filter the output
        filtered_output = filter_output(output)

        # Check if simulation has ended
        if "Simulation ended" in output:
            step_proc.terminate()
            step_proc = None
            return {"output": filtered_output, "registers": None, "memory": None, "simulationEnded": True}

        return {"output": filtered_output, "registers": None, "memory": None, "simulationEnded": False}

    except Exception as e:
        if step_proc:
            step_proc.terminate()
            step_proc = None
        return {"output": f"Step error: {str(e)}", "registers": None, "memory": None, "simulationEnded": True}

@app.post("/compile")
async def compile_code(request: Request):
    try:
        data = await request.json()
        code = data.get("code", "")

        # Create unique binary file name
        bin_file = os.path.join(current_dir, f"temp_{uuid.uuid4()}.bin")

        # Write assembly code to temporary file
        asm_file = bin_file + ".s"
        with open(asm_file, "w") as f:
            f.write(code)

        # Assemble the code
        assembler = ZX16Assembler()
        try:
            with open(asm_file, "r") as f:
                assembly_code = f.read()
            binary_code = assembler.assemble(assembly_code)
            with open(bin_file, "wb") as f:
                f.write(binary_code)
        except Exception as e:
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return {"output": f"Assembly error: {str(e)}", "registers": None, "memory": None}
        finally:
            if os.path.exists(asm_file):
                os.remove(asm_file)

        # Terminate previous process if running
        if step_proc and step_proc.poll() is None:
            step_proc.terminate()
        step_proc = None

        # Start persistent simulator process in step mode
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        step_proc = subprocess.Popen(
            [simulator_path, bin_file, "step"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=current_dir,
            text=True,
            bufsize=1
        )
        latest_step_bin = bin_file
        step_count = 0
        prev_registers = {}
        prev_memory = []

        # Read and filter initial output
        initial_output = step_proc.stdout.readline()
        filtered_output = filter_output(initial_output)

        return {"output": filtered_output, "registers": None, "memory": None}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}
