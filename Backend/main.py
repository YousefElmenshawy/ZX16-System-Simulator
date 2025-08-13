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
<<<<<<< Updated upstream
    for addr in range(0, 256, 16):
=======
    for addr in range(0, 65536, 16):
>>>>>>> Stashed changes
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
            output += f"Assembly failed.\nStderr:\n{result.stderr}\nStdout:\n{result.stdout}"
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

<<<<<<< Updated upstream
=======
        # Clean up temp files ASAP
>>>>>>> Stashed changes
        if os.path.exists(asm_file):
            os.remove(asm_file)
        if os.path.exists(bin_file):
            os.remove(bin_file)

        if result.returncode != 0:
            output += f"Simulation failed.\nStderr:\n{result.stderr}\nStdout:\n{result.stdout}"
            return {"output": output, "registers": None, "memory": None}
        else:
            output += f"Simulation succeeded.\nStdout:\n{result.stdout}"
            if result.stderr:
                output += f"\nStderr:\n{result.stderr}"

        default_registers = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
        default_memory = []
<<<<<<< Updated upstream
        for addr in range(0, 256, 16):
=======
        for addr in range(0, 65536, 16):
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
        mem_size = 256
=======
        mem_size = 65536
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
                    addr = addr.strip()
                    bytes_list = [b for b in bytestr.strip().split(' ') if b]
                    for mrow in memory:
                        if mrow["address"].lower() == f"0x{int(addr,16):04x}":
                            mrow["bytes"] = bytes_list
                            break
=======
                    addr_int = int(addr.strip(), 16)
                    bytes_list = bytestr.strip().split()
                    idx = addr_int // 16
                    if 0 <= idx < len(memory):
                        memory[idx]["bytes"] = bytes_list
            elif line_strip.startswith("Memory Dump"):
                mem_section = True

>>>>>>> Stashed changes
        prev_memory = [dict(m) for m in memory]
        prev_registers = registers.copy()  # <-- Update prev_registers here

        # Identify changed registers (for highlighting, optional)
<<<<<<< Updated upstream
=======

        # (No previous state to compare in run mode, so leave empty or compare to default_registers if desired)

        # Identify changed registers (for highlighting, optional)
>>>>>>> Stashed changes
        changed_registers = []
        # (No previous state to compare in run mode, so leave empty or compare to default_registers if desired)

        return {"output": output, "registers": registers, "memory": prev_memory, "changed_registers": changed_registers}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

@app.post("/simulate_step")
async def simulate_step(request: Request):
<<<<<<< Updated upstream
    """
    Assemble code and prepare for step mode.
    """
=======
>>>>>>> Stashed changes
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

<<<<<<< Updated upstream
        if latest_step_bin and os.path.exists(latest_step_bin):
            os.remove(latest_step_bin)
        latest_step_bin = bin_file
        step_count = 0  # Reset step count on new code
        prev_registers = {}  # Reset previous registers
        prev_memory = []  # Reset previous memory
=======
        # Terminate previous process if running
>>>>>>> Stashed changes
        if step_proc and step_proc.poll() is None:
            step_proc.terminate()
        step_proc = None
        step_output_buffer = ""

<<<<<<< Updated upstream
        return {"output": "Assembly succeeded. Ready for step mode.", "registers": None, "memory": None}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

@app.post("/step")
async def step_simulation():
    """
    Perform a single step in the simulator and return state.
    """
    global latest_step_bin, step_count, prev_registers, prev_memory
    try:
        if not latest_step_bin or not os.path.exists(latest_step_bin):
            return {"output": "No assembled binary available for step mode.", "registers": None, "memory": None}

        current_dir = os.path.dirname(os.path.abspath(__file__))
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")

        step_count += 1  # Increment step count

        # Run the simulator with step count as parameter
        proc = subprocess.Popen(
            [simulator_path, latest_step_bin, "step", str(step_count)],
=======
        # Start persistent simulator process in step mode
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        step_proc = subprocess.Popen(
            [simulator_path, bin_file, "step"],
>>>>>>> Stashed changes
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=current_dir,
            text=True,
            bufsize=1
        )
<<<<<<< Updated upstream
        proc.stdin.close()  # No need to send newlines, step count is passed as argument

        # Read all output
        output = ""
        timeout = 2.0
        start_time = time.time()
        while True:
            line = proc.stdout.readline()
            if line:
                output += line
                if "Simulation ended." in line:
                    break
            else:
                if time.time() - start_time > timeout:
                    break
                time.sleep(0.05)
        error_output = proc.stderr.read()
        if error_output:
            output += error_output

        proc.stdout.close()
        proc.stderr.close()
        proc.wait(timeout=2)

        # If simulation ended, reset step count
        if "Simulation ended." in output:
            step_count = 0

        # Parse registers and memory as before
        default_registers = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
        default_memory = []
        for addr in range(0, 256, 16):
=======
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
        # Do not expect or use code from the request
        if not step_proc or step_proc.poll() is not None:
            return {"output": "Step mode not active or process ended.", "registers": None, "memory": None}

        # Send a newline to stdin to trigger a step
        step_proc.stdin.write("\n")
        step_proc.stdin.flush()

        # Read all output from the step, including register state and memory dump
        output = ""
        timeout = 5.0  # Increased timeout for memory dump
        start_time = time.time()

        # Read until we get all output (registers + memory dump)
        registers_found = False
        memory_dump_started = False
        memory_dump_ended = False

        while True:
            try:
                line = step_proc.stdout.readline()
                if not line:
                    break
                output += line

                # Track what sections we've seen
                if "Current PC:" in line:
                    registers_found = True
                elif "Memory Dump" in line:
                    memory_dump_started = True
                elif memory_dump_started and line.strip() == "":
                    # Empty line after memory dump indicates end
                    memory_dump_ended = True

                # Check for simulation end
                if "Simulation ended." in line or "PC out of program bounds" in line:
                    # Read a bit more to get final state and memory
                    for _ in range(4100):  # Enough for full memory dump
                        try:
                            extra_line = step_proc.stdout.readline()
                            if not extra_line:
                                break
                            output += extra_line
                        except:
                            break
                    break

                # If we've seen registers and completed memory dump, we're done
                if registers_found and memory_dump_ended:
                    break

                # Add timeout to prevent hanging
                if time.time() - start_time > timeout:
                    break
            except:
                break

        # Check if simulation ended
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
>>>>>>> Stashed changes
            default_memory.append({
                "address": f"0x{addr:04X}",
                "bytes": ["00"] * 16
            })
<<<<<<< Updated upstream
        registers = default_registers.copy()
        memory = [dict(m) for m in default_memory]

        # Extract current PC value more reliably
        current_pc = None
        pc_pattern = re.compile(r"Current PC: 0x([0-9a-fA-F]+)")
        for line in output.splitlines():
            pc_match = pc_pattern.search(line)
            if pc_match:
                current_pc = pc_match.group(1)
                break

        # Extract instruction being executed
        current_instruction = None
        for line in output.splitlines():
            if line.startswith('[') and ']' in line and 'Executed:' not in line:
                try:
                    instruction_parts = line.split('  ', 2)
                    if len(instruction_parts) >= 3:
                        current_instruction = instruction_parts[2].strip()
                        break
                except:
                    pass

        # Improved register and memory parsing
        lines = output.splitlines()
        reg_section = False
        mem_section = False
        for line in lines:
            line = line.strip()
            for reg in registers.keys():
                if line.startswith(reg + " ="):
                    reg_val = line.split('=')[1].strip()
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
            # Memory parsing
            if line.startswith("Memory Dump"):
                mem_section = True
                continue
            if mem_section:
                if line == '':
                    mem_section = False
                    continue
                if ':' in line:
                    addr, bytestr = line.split(':', 1)
                    bytes_list = [b for b in bytestr.strip().split(' ') if b]
                    for m in memory:
                        if m["address"].lower() == f"0x{int(addr,16):04x}":
                            m["bytes"] = bytes_list
                            break
=======
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
>>>>>>> Stashed changes

        # Identify changed registers
        changed_registers = []
        if prev_registers:
            for reg, val in registers.items():
                if reg in prev_registers and prev_registers[reg] != val:
                    changed_registers.append(reg)
        prev_registers = registers.copy()

<<<<<<< Updated upstream
        # Always update prev_memory to reflect the latest memory after each step
        prev_memory = [dict(m) for m in memory]

        return {
            "output": output,
            "registers": registers,
            "memory": prev_memory,
            "changed_registers": changed_registers,
=======
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
        current_pc = None
        current_instruction = None
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
            "output": output,
            "registers": registers,
            "memory": memory,
            "changed_registers": changed_registers,
            "changed_memory": changed_memory,
>>>>>>> Stashed changes
            "current_pc": current_pc,
            "current_instruction": current_instruction
        }
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}
