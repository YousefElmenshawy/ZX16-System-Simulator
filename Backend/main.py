from fastapi import FastAPI, Request
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
import subprocess
import uuid
import os
from fastapi.responses import JSONResponse
import time
import re

app = FastAPI()
app.mount("/", StaticFiles(directory="Frontend", html=True), name="frontend")
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
instruction_log = []  # Global instruction log that persists across steps

# --- NEW: Run-mode interactive process state ---
latest_run_bin = None
run_proc = None

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
    # Rework to run simulator interactively so ECALL prompts can be handled by frontend
    global prev_memory, prev_registers, run_proc, latest_run_bin
    try:
        data = await request.json()
        code = data["code"]

        uid = str(uuid.uuid4())
        current_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.abspath(os.path.join(current_dir, os.pardir))
        asm_file = os.path.join(current_dir, f"temp_{uid}.s")
        bin_file = os.path.join(current_dir, f"temp_{uid}.bin")

        with open(asm_file, "w", encoding="utf-8") as f:
            f.write(code)

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
            error_output = "Assembly failed.\n"
            if result.stderr.strip():
                error_output += f"Errors:\n{result.stderr.strip()}\n"
            if result.stdout.strip():
                error_output += f"Output:\n{result.stdout.strip()}\n"
            if os.path.exists(bin_file):
                os.remove(bin_file)
            return {"output": error_output, "registers": None, "memory": None}

        # Terminate any prior run process
        if run_proc and run_proc.poll() is None:
            try:
                run_proc.terminate()
            except Exception:
                pass
            run_proc = None
        # Start simulator in interactive run mode with stdin/stdout pipes
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        # Use project root as CWD when audio assets exist there so relative loads work
        working_dir = project_root if os.path.exists(os.path.join(project_root, "ballhit.wav")) else current_dir
        run_proc = subprocess.Popen(
            [simulator_path, bin_file],  # bin_file is absolute
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=working_dir,
            text=True,
            bufsize=1
        )
        latest_run_bin = bin_file

        # Read output until we hit an ECALL input prompt or the program ends
        output = ""
        timeout = 10.0
        start_time = time.time()
        awaiting_input = False
        while True:
            line = run_proc.stdout.readline()
            if not line:
                if run_proc.poll() is not None:
                    # Process has ended, read any remaining output
                    remaining_output = run_proc.stdout.read()
                    if remaining_output:
                        output += remaining_output
                    break
                if time.time() - start_time > timeout:
                    break
                continue
            output += line
            low = line.lower()

            # Stop as soon as the user-visible prompt appears
            if ('enter string:' in low) or ('enter number:' in low) or ('press key:' in low):
                awaiting_input = True
                break

            # If simulation ends, read remaining output to capture final state
            if ('simulation ended.' in low) or ('pc out of program bounds' in low) or ('program exiting' in low):
                # Read remaining output for a bit longer to capture register dumps and memory dumps
                additional_timeout = 2.0
                additional_start = time.time()
                while time.time() - additional_start < additional_timeout:
                    extra_line = run_proc.stdout.readline()
                    if not extra_line:
                        # Check if process ended and read any buffered output
                        if run_proc.poll() is not None:
                            remaining = run_proc.stdout.read()
                            if remaining:
                                output += remaining
                        break
                    output += extra_line
                break

            if time.time() - start_time > timeout:
                break

        filtered_output = filter_output(output)

        # If process has exited or we saw final message, cleanup temp bin
        simulation_ended = (run_proc is None) or (run_proc.poll() is not None) or ("Simulation ended." in output)

        # IMPORTANT: If awaiting input, return last known state to preserve UI tables
        if awaiting_input:
            # Use previous state if available; otherwise, return zeroed defaults
            if prev_registers:
                regs = prev_registers.copy()
            else:
                regs = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
            if prev_memory:
                mem = [dict(m) for m in prev_memory]
            else:
                mem = []
                for addr in range(0, 65536, 16):
                    mem.append({"address": f"0x{addr:04X}", "bytes": ["00"] * 16})
            return {"output": filtered_output, "registers": regs, "memory": mem, "simulationEnded": simulation_ended}

        # Parse registers and memory from the output (always, not just when simulation ends)
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

        # Parse memory from the output
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
            if line_strip.startswith("Memory Dump"):
                mem_section = True
                continue
            if mem_section:
                if line_strip == '':
                    mem_section = False
                    continue
                if ':' in line_strip:
                    addr, bytestr = line_strip.split(':', 1)
                    try:
                        addr_int = int(addr.strip(), 16)
                        bytes_list = bytestr.strip().split()
                        idx = addr_int // 16
                        if 0 <= idx < len(memory):
                            memory[idx]["bytes"] = bytes_list
                    except ValueError:
                        continue

        # Update previous state snapshots for future ECALL prompts
        prev_registers = registers.copy()
        prev_memory = [dict(m) for m in memory]

        if simulation_ended:
            try:
                if run_proc and run_proc.poll() is None:
                    run_proc.terminate()
            finally:
                run_proc = None
            if os.path.exists(latest_run_bin or ""):
                try:
                    os.remove(latest_run_bin)
                except Exception:
                    pass
                latest_run_bin = None

        return {"output": filtered_output, "registers": registers, "memory": memory, "simulationEnded": simulation_ended}
    except Exception as e:
        try:
            if run_proc and run_proc.poll() is None:
                run_proc.terminate()
        except Exception:
            pass
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

@app.post("/simulate_step")
async def simulate_step(request: Request):
    global latest_step_bin, step_proc, step_output_buffer, step_count, prev_registers, prev_memory, instruction_log
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
            # Don't filter assembly errors - show them in full detail
            error_output = "Assembly failed.\n"
            if result.stderr.strip():
                error_output += f"Errors:\n{result.stderr.strip()}\n"
            if result.stdout.strip():
                error_output += f"Output:\n{result.stdout.strip()}\n"
            if os.path.exists(bin_file):
                os.remove(bin_file)
            return {"output": error_output, "registers": None, "memory": None}

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
        instruction_log = []  # Reset instruction log for new program

        return {"output": "Assembly succeeded. Ready for step mode.", "registers": None, "memory": None}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

@app.post("/step")
async def step_simulation():
    global latest_step_bin, step_count, prev_registers, prev_memory, step_proc, instruction_log
    try:
        if not step_proc or step_proc.poll() is not None:
            return {"output": "Step mode not active or process ended.", "registers": None, "memory": None, "mode": "step"}

        # Trigger one simulator step
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
        ecall_seen = False  # Track ECALL context
        step_logged = False  # Ensure only one log entry per step
        step_completed = False  # Mark real step completion for correct counting
        awaiting_input = False  # NEW: detect ECALL prompt and avoid zeroing state

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
                        # Mark step completion
                        step_completed = True
                        # Add to global instruction log exactly once per step
                        if not step_logged:
                            instruction_log.append({
                                "pc": current_pc if current_pc else "",
                                "instruction": current_instruction,
                                "step": step_count + 1
                            })
                            step_logged = True
                elif line.strip().startswith("[") and "0x" in line and not "Executed:" in line:
                    # Parse disassembly line format: [0020]  0x1234  ADD t0, t1
                    disasm_match = re.match(r'\[([0-9a-fA-F]+)\]\s+0x([0-9a-fA-F]+)\s+(.+)', line.strip())
                    if disasm_match:
                        pc_addr = disasm_match.group(1)
                        instruction_text = disasm_match.group(3).strip()
                        # Mark step completion (we saw the instruction line)
                        step_completed = True
                        if not step_logged:
                            instruction_log.append({
                                "pc": pc_addr,
                                "instruction": instruction_text,
                                "step": step_count + 1
                            })
                            step_logged = True
                elif "ECALL executed" in line:
                    # Consider ECALL as a completed step even if it prompts for input next
                    ecall_seen = True
                    step_completed = True
                elif "Enter string:" in line or "Enter number:" in line or "Press key:" in line:
                    # We've reached the user prompt; stop and let frontend prompt
                    registers_found = True
                    memory_dump_ended = True
                    awaiting_input = True
                    break
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

        # Increment step count only if a step truly completed
        if step_completed:
            step_count += 1

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
                    try:
                        addr_int = int(addr.strip(), 16)
                        bytes_list = bytestr.strip().split()
                        idx = addr_int // 16
                        if 0 <= idx < len(memory):
                            memory[idx]["bytes"] = bytes_list
                    except ValueError:
                        continue

        # If awaiting ECALL input, do NOT overwrite previous snapshots; return previous state
        if awaiting_input:
            # Use previous state if available; otherwise, return zeroed defaults
            if prev_registers:
                registers = prev_registers.copy()
            if prev_memory:
                memory = [dict(m) for m in prev_memory]

            return {
                "output": filtered_output,
                "registers": registers,
                "memory": memory,
                "changed_registers": [],
                "changed_memory": [],
                "current_pc": current_pc,
                "current_instruction": current_instruction,
                "instruction_log": instruction_log,
                "simulationEnded": simulation_ended,
                "mode": "step"
            }

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
            "output": filtered_output,
            "registers": registers,
            "memory": memory,
            "changed_registers": changed_registers,
            "changed_memory": changed_memory,
            "current_pc": current_pc,
            "current_instruction": current_instruction,
            "instruction_log": instruction_log,  # Return the global accumulated log
            "simulationEnded": simulation_ended,
            "mode": "step"
        }
    except Exception as e:
        if step_proc:
            step_proc.terminate()
            step_proc = None
        return {"output": f"Step error: {str(e)}", "registers": None, "memory": None, "simulationEnded": True, "mode": "step"}

@app.post("/ecall_input")
async def handle_ecall_input(request: Request):
    global step_proc, run_proc, latest_run_bin, prev_registers, prev_memory, instruction_log, step_count
    try:
        # Select active process and mode
        if step_proc and step_proc.poll() is None:
            active_proc = step_proc
            mode = 'step'
        elif run_proc and run_proc.poll() is None:
            active_proc = run_proc
            mode = 'run'
        else:
            return {"output": "No active simulation process for ECALL input.", "registers": None, "memory": None}

        data = await request.json()
        service = data.get("service")
        user_input = data.get("input", "")

        # Send input
        if service == 1:
            active_proc.stdin.write(user_input + "\n")
        elif service == 2:
            active_proc.stdin.write(str(user_input) + "\n")
        elif service == 7:
            ch = user_input[0] if user_input else '\n'
            active_proc.stdin.write(ch + "\n")
        else:
            return {"output": f"Unsupported ECALL service: {service}", "registers": None, "memory": None}
        active_proc.stdin.flush()

        # Read output with improved parsing
        output = ""
        start = time.time()
        timeout = 30.0
        ecall_done = False
        simulation_ended = False

        # Track state parsing indicators
        found_registers = False
        found_memory = False
        state_output_started = False

        while True:
            if time.time() - start > timeout:
                break
            line = active_proc.stdout.readline()
            if not line:
                if active_proc.poll() is not None:
                    break
                time.sleep(0.01)
                continue
            output += line
            low = line.lower().strip()
            stripped = line.strip()

            # End conditions
            if 'simulation ended.' in low or 'pc out of program bounds' in low:
                simulation_ended = True
                # Read remaining output
                drain_t = time.time()
                while time.time() - drain_t < 2.0:
                    extra = active_proc.stdout.readline()
                    if not extra:
                        break
                    output += extra
                break

            # Track ECALL completion
            if 'ecall done. continuing the simulator.' in low or 'ecall done' in low:
                ecall_done = True
                state_output_started = True
                continue

            # After ECALL done, look for state output
            if state_output_started:
                # Check for register output (lines like "t0 = 5")
                if any(stripped.startswith(reg + " =") for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]):
                    found_registers = True

                # Check for memory dump start
                if stripped.startswith("Memory Dump"):
                    found_memory = True

                # If we found both registers and memory in step mode, we can break
                if mode == 'step' and found_registers and found_memory:
                    # Read a bit more to ensure we get complete memory dump
                    extra_timeout = time.time() + 1.0
                    while time.time() < extra_timeout:
                        extra_line = active_proc.stdout.readline()
                        if not extra_line:
                            break
                        output += extra_line
                        if extra_line.strip() == "":  # Empty line usually ends memory dump
                            break
                    break

            # Check for new input prompts
            if 'enter string:' in low or 'enter number:' in low or 'press key:' in low:
                break





        filtered = filter_output(output)

        # Cleanup run proc if ended
        if mode == 'run' and (simulation_ended or (run_proc and run_proc.poll() is not None)):
            try:
                if run_proc and run_proc.poll() is None:
                    run_proc.terminate()
            finally:
                run_proc = None
            if os.path.exists(latest_run_bin or ""):
                try:
                    os.remove(latest_run_bin)
                except Exception:
                    pass
                latest_run_bin = None

        # Parse state with improved error handling
        registers = parse_registers_from_output(output)
        memory = parse_memory_from_output(output)

        # CRITICAL FIX: If parsing failed, use previous state to maintain continuity
        if not registers or all(v == 0 for v in registers.values()):
            if prev_registers:
                registers = prev_registers.copy()
                print(f"[DEBUG] Using previous registers state: {registers}")
            else:
                registers = {reg: 0 for reg in ["t0","ra","sp","s0","s1","t1","a0","a1"]}

        if not memory or all(all(b == "00" for b in row["bytes"]) for row in memory):
            if prev_memory:
                memory = [dict(m) for m in prev_memory]
                print(f"[DEBUG] Using previous memory state (first row): {memory[0] if memory else 'empty'}")
            else:
                memory = [{"address": f"0x{addr:04X}", "bytes": ["00"]*16} for addr in range(0,65536,16)]

        # Update previous state snapshots
        if registers and any(v != 0 for v in registers.values()):
            prev_registers = registers.copy()
        if memory and any(any(b != "00" for b in row["bytes"]) for row in memory):
            prev_memory = [dict(m) for m in memory]

        # Parse additional state info
        current_pc = parse_current_pc_from_output(output)
        current_instruction = parse_current_instruction_from_output(output)

        # Track register changes
        changed_registers = []
        if prev_registers:
            for r,v in registers.items():
                if r in prev_registers and prev_registers[r] != v:
                    changed_registers.append(r)

        # Track memory changes
        changed_memory = []
        if prev_memory:
            for i, row in enumerate(memory):
                if i < len(prev_memory) and prev_memory[i]["bytes"] != row["bytes"]:
                    diffs = [j for j,(a,b) in enumerate(zip(prev_memory[i]["bytes"], row["bytes"])) if a!=b]
                    if diffs:
                        changed_memory.append({"row_index": i, "address": row["address"], "changed_bytes": diffs})

        return {
            "output": filtered,
            "registers": registers,
            "memory": memory,
            "changed_registers": changed_registers,
            "changed_memory": changed_memory,
            "current_pc": current_pc,
            "current_instruction": current_instruction,
            "instruction_log": instruction_log,
            "ecall_completed": ecall_done,
            "simulationEnded": simulation_ended,
            "mode": mode  # Add mode info for frontend
        }
    except Exception as e:
        print(f"[ERROR] ECALL input error: {str(e)}")
        return {"output": f"ECALL input error: {str(e)}", "registers": None, "memory": None}

# Add helper functions for improved parsing
def parse_registers_from_output(output):
    """Parse register values from C++ simulator output"""
    registers = {reg: 0 for reg in ["t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"]}
    lines = output.splitlines()

    for line in lines:
        line_strip = line.strip()
        # Look for register format: "t0 = 5"
        for reg in registers.keys():
            if line_strip.startswith(reg + " ="):
                try:
                    reg_val = line_strip.split('=')[1].strip()
                    val = int(reg_val)
                    # Handle signed 16-bit values
                    if val >= 2**15:
                        val -= 2**16
                    registers[reg] = val
                except (ValueError, IndexError):
                    pass
                break

    return registers

def parse_memory_from_output(output):
    """Parse memory dump from C++ simulator output"""
    # Initialize default memory structure
    default_memory = []
    for addr in range(0, 65536, 16):
        default_memory.append({
            "address": f"0x{addr:04X}",
            "bytes": ["00"] * 16
        })
    memory = [dict(m) for m in default_memory]

    lines = output.splitlines()
    mem_section = False

    for line in lines:
        line_strip = line.strip()

        # Detect start of memory dump
        if line_strip.startswith("Memory Dump"):
            mem_section = True
            continue

        if mem_section:
            # Empty line ends memory section
            if line_strip == '':
                mem_section = False
                continue

            # Parse memory line format: "0020: 05 00 1c 00 05 00 00 00 01 00 05 00 04 00 00 00"
            if ':' in line_strip:
                try:
                    addr_part, bytestr = line_strip.split(':', 1)
                    addr_int = int(addr_part.strip(), 16)
                    bytes_list = bytestr.strip().split()

                    # Find the corresponding row in our memory structure
                    idx = addr_int // 16
                    if 0 <= idx < len(memory):
                        # Ensure we have the right number of bytes (pad with 00 if needed)
                        while len(bytes_list) < 16:
                            bytes_list.append("00")
                        memory[idx]["bytes"] = bytes_list[:16]  # Take only first 16 bytes
                except (ValueError, IndexError):
                    continue

    return memory

def parse_current_pc_from_output(output):
    """Parse current PC from C++ simulator output"""
    lines = output.splitlines()
    for line in lines:
        if "Current PC:" in line:
            pc_match = re.search(r"0x([0-9a-fA-F]+)", line)
            if pc_match:
                return pc_match.group(1)
    return None

def parse_current_instruction_from_output(output):
    """Parse current instruction from C++ simulator output"""
    lines = output.splitlines()
    for line in lines:
        if "Executed:" in line:
            inst_match = re.search(r"Executed:\s*(.+)", line)
            if inst_match:
                return inst_match.group(1).strip()
    return None

def filter_output(text):
    if not text:
        return ""
    filtered_lines = []
    lines = text.split('\n')

    # Track if we're in an ECALL context
    in_ecall_context = False
    simulation_ended = False

    for line in lines:
        line_lower = line.lower()

        # Check if simulation has ended - stop processing after this
        if 'simulation ended' in line_lower:
            filtered_lines.append(line)
            simulation_ended = True
            break  # Don't process any more lines after "Simulation ended."


        # Check if program is exiting - stop processing after this line
        if 'program exiting' in line_lower:
            #filtered_lines.append(line)
            break  # Don't process any more lines after "Program exiting..."

        # Skip processing if simulation has already ended
        if simulation_ended:
            continue

        # Check if this line starts an ECALL context
        if 'ecall executed' in line_lower:
            in_ecall_context = True
            #filtered_lines.append(line)
            continue

        # Check if ECALL context ends
        if 'ecall done' in line_lower:
            #filtered_lines.append(line)
            in_ecall_context = False
            continue

        # Only show lines that are between "ECALL executed" and "ECALL done"
        if in_ecall_context:
            filtered_lines.append(line)
            continue

        # Show general simulation messages (outside ECALL context)
        if ('simulation' in line_lower):
            filtered_lines.append(line)
            continue

    return '\n'.join(filtered_lines)

@app.post("/step_raw")
async def step_code_raw(request: Request):
    global step_proc, latest_step_bin, step_count, prev_registers, prev_memory
    try:
        if not step_proc or step_proc.poll() is not None:
            return {"output": "No active simulation. Please compile and run first.", "registers": None, "memory": None}

        # Send newline to trigger next step
        step_proc.stdin.write('\n')
        step_proc.stdin.flush()

        # Read output until we get a complete step
        output = ""
        seen_ecall = False
        start_time = time.time()
        timeout = 5.0
        while True:
            line = step_proc.stdout.readline()
            if not line:
                if time.time() - start_time > timeout:
                    break
                continue
            output += line
            low = line.lower()

            if 'ecall executed' in low:
                seen_ecall = True
                # do not break; wait for actual prompt
                continue

            if ('enter string:' in low) or ('enter number:' in low) or ('press key:' in low):
                break

            if 'simulation ended' in low or 'pc out of program bounds' in low:
                break

            if not seen_ecall and 'executed' in low:
                break

            if time.time() - start_time > timeout:
                break

        filtered_output = filter_output(output)

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
    # Re-implement compile to use external assembler like other endpoints and start step-mode
    global step_proc, latest_step_bin, step_count, prev_registers, prev_memory
    try:
        data = await request.json()
        code = data.get("code", "")

        uid = str(uuid.uuid4())
        current_dir = os.path.dirname(os.path.abspath(__file__))
        asm_file = os.path.join(current_dir, f"temp_{uid}.s")
        bin_file = os.path.join(current_dir, f"temp_{uid}.bin")

        with open(asm_file, "w") as f:
            f.write(code)

        # Assemble via zx16asm.py
        assemble_cmd = ["python", os.path.join(current_dir, "zx16asm.py"), asm_file, "-o", bin_file]
        try:
            result = subprocess.run(assemble_cmd, capture_output=True, text=True, cwd=current_dir, timeout=30)
        except subprocess.TimeoutExpired:
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return {"output": "Assembly process timed out!", "registers": None, "memory": None}

        if os.path.exists(asm_file):
            os.remove(asm_file)

        if result.returncode != 0:
            error_output = "Assembly failed.\n"
            if result.stderr.strip():
                error_output += f"Errors:\n{result.stderr.strip()}\n"
            if result.stdout.strip():
                error_output += f"Output:\n{result.stdout.strip()}\n"
            if os.path.exists(bin_file):
                os.remove(bin_file)
            return {"output": error_output, "registers": None, "memory": None}

        # Terminate previous process if running
        if step_proc and step_proc.poll() is None:
            step_proc.terminate()
        step_proc = None

        # Start simulator in step mode using compiled binary
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

        # Read first line to confirm readiness (non-blocking style with small timeout)
        start_time = time.time()
        initial_output = ""
        while time.time() - start_time < 0.25:
            try:
                line = step_proc.stdout.readline()
                if not line:
                    break
                initial_output += line
                if "ECALL" in line or "Simulation" in line:
                    break
            except Exception:
                break
        filtered_output = filter_output(initial_output) if initial_output else "Assembly succeeded. Ready for step mode."

        return {"output": filtered_output, "registers": None, "memory": None}
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "registers": None, "memory": None}

# --- New Interactive Graphics Run Endpoint ---
@app.post("/run_interactive")
async def run_interactive(request: Request):
    global run_proc, step_proc, latest_run_bin
    try:
        data = await request.json()
        code = data.get("code", "")
        if not code.strip():
            return {"output": "No code provided.", "started": False}

        # Paths
        current_dir = os.path.dirname(os.path.abspath(__file__))  # Backend dir
        project_root = os.path.abspath(os.path.join(current_dir, os.pardir))
        asm_uid = str(uuid.uuid4())
        asm_file = os.path.join(current_dir, f"temp_{asm_uid}.s")
        bin_file = os.path.join(current_dir, f"temp_{asm_uid}.bin")

        # Write source
        with open(asm_file, "w", encoding="utf-8") as f:
            f.write(code)

        # Assemble
        assemble_cmd = [
            "python",
            os.path.join(current_dir, "zx16asm.py"),
            asm_file,
            "-o",
            bin_file,
        ]
        try:
            result = subprocess.run(
                assemble_cmd,
                capture_output=True,
                text=True,
                cwd=current_dir,
                timeout=30,
            )
        except subprocess.TimeoutExpired:
            try:
                if os.path.exists(asm_file):
                    os.remove(asm_file)
            finally:
                return {"output": "Assembly process timed out!", "started": False}
        finally:
            # Always remove the temporary asm file
            if os.path.exists(asm_file):
                try:
                    os.remove(asm_file)
                except Exception:
                    pass

        if result.returncode != 0 or not os.path.exists(bin_file):
            error_output = "Assembly failed.\n"
            if result.stderr and result.stderr.strip():
                error_output += f"Errors:\n{result.stderr.strip()}\n"
            if result.stdout and result.stdout.strip():
                error_output += f"Output:\n{result.stdout.strip()}\n"
            if os.path.exists(bin_file):
                try:
                    os.remove(bin_file)
                except Exception:
                    pass
            return {"output": error_output, "started": False}

        # Terminate any running processes to avoid conflicts
        try:
            if run_proc and run_proc.poll() is None:
                run_proc.terminate()
        except Exception:
            pass
        run_proc = None
        try:
            if step_proc and step_proc.poll() is None:
                step_proc.terminate()
        except Exception:
            pass
        step_proc = None

        # Prefer running with CWD at project root so relative game assets (e.g. wav files) load
        # but keep absolute path to simulator executable in Backend
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        working_dir = project_root if os.path.exists(os.path.join(project_root, "ballhit.wav")) else current_dir

        # Windows: detach to avoid inheriting console; other OS: normal spawn
        creationflags = 0
        if os.name == "nt":
            CREATE_NEW_PROCESS_GROUP = 0x00000200
            DETACHED_PROCESS = 0x00000008
            creationflags = CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS

        try:
            run_proc = subprocess.Popen(
                [simulator_path, bin_file, "interactive"],
                stdin=subprocess.DEVNULL,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                cwd=working_dir,
                creationflags=creationflags,
            )
        except FileNotFoundError:
            # Cleanup bin if simulator missing
            try:
                if os.path.exists(bin_file):
                    os.remove(bin_file)
            except Exception:
                pass
            return {"output": "Simulator executable not found.", "started": False}
        except Exception as e:
            # Cleanup bin on failure
            try:
                if os.path.exists(bin_file):
                    os.remove(bin_file)
            except Exception:
                pass
            return {"output": f"Failed to start interactive simulator: {e}", "started": False}

        latest_run_bin = bin_file  # Keep track for later cleanup

        # Quick health check: if process exits immediately, report failure
        time.sleep(0.25)
        if run_proc.poll() is not None:
            rc = run_proc.returncode
            # Clean up bin on immediate exit to avoid pile-up
            try:
                if os.path.exists(latest_run_bin or ""):
                    os.remove(latest_run_bin)
            except Exception:
                pass
            latest_run_bin = None
            run_proc = None
            return {"output": f"Interactive simulator exited immediately (code {rc}).", "started": False}

        return {
            "output": "Interactive graphics run started. Close the window to end.",
            "started": True,
            "mode": "interactive",
        }
    except Exception as e:
        return {"output": f"Server error: {str(e)}", "started": False}

@app.post("/stop")
async def stop_simulation():
    global run_proc, step_proc, latest_run_bin, latest_step_bin, prev_registers, prev_memory
    try:
        mode = None
        collected_output = ""
        # Prefer stopping normal run process
        if run_proc and run_proc.poll() is None:
            mode = 'run'
            try:
                run_proc.terminate()
            except Exception:
                pass
            # Try to collect any remaining output briefly
            try:
                out, err = run_proc.communicate(timeout=1)
                collected_output += (out or "")
            except Exception:
                pass
            finally:
                run_proc = None
            # Cleanup latest run bin
            if os.path.exists(latest_run_bin or ""):
                try:
                    os.remove(latest_run_bin)
                except Exception:
                    pass
                latest_run_bin = None
        # Optionally stop step process too (no-op if not running)
        elif step_proc and step_proc.poll() is None:
            mode = 'step'
            try:
                step_proc.terminate()
            except Exception:
                pass
            try:
                out, err = step_proc.communicate(timeout=1)
                collected_output += (out or "")
            except Exception:
                pass
            finally:
                step_proc = None
        else:
            # Nothing to stop
            regs = prev_registers.copy() if prev_registers else {reg: 0 for reg in ["t0","ra","sp","s0","s1","t1","a0","a1"]}
            mem = [dict(m) for m in prev_memory] if prev_memory else [{"address": f"0x{addr:04X}", "bytes": ["00"]*16} for addr in range(0,65536,16)]
            return {
                "output": "No active simulation process.\nSimulation ended.",
                "registers": regs,
                "memory": mem,
                "simulationEnded": True,
                "mode": mode or 'run'
            }

        # Build response using last known state to avoid blanks
        regs = prev_registers.copy() if prev_registers else {reg: 0 for reg in ["t0","ra","sp","s0","s1","t1","a0","a1"]}
        mem = [dict(m) for m in prev_memory] if prev_memory else [{"address": f"0x{addr:04X}", "bytes": ["00"]*16} for addr in range(0,65536,16)]

        # Indicate user stop; include Simulation ended. for frontend logic
        final_output = (collected_output or "").strip()
        if final_output:
            final_output += "\n"
        final_output += "Simulation ended. (stopped by user)"

        return {
            "output": filter_output(final_output),
            "registers": regs,
            "memory": mem,
            "simulationEnded": True,
            "mode": mode or 'run'
        }
    except Exception as e:
        return {"output": f"Stop error: {str(e)}", "registers": None, "memory": None, "simulationEnded": True}

