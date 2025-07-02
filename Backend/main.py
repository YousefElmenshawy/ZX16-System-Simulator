from fastapi import FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
import subprocess
import uuid
import os
from fastapi.responses import JSONResponse

app = FastAPI()

# Add CORS middleware configuration
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://127.0.0.1:8080"],  # Allow frontend origin
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/simulate")
async def simulate_code(request: Request):
    try:
        data = await request.json()
        code = data["code"]

        # Create unique temp files
        uid = str(uuid.uuid4())
        asm_file = f"temp_{uid}.s"
        bin_file = f"temp_{uid}.bin"

        # Write assembly code to file
        with open(asm_file, "w") as f:
            f.write(code)

        # Get current directory
        current_dir = os.path.dirname(os.path.abspath(__file__))
        print(f"[DEBUG] Current directory: {current_dir}")
        print(f"[DEBUG] Assembly file: {asm_file}, Binary file: {bin_file}")

        # Assemble
        assemble_cmd = ["python3", os.path.join(current_dir, "zx16asm.py"), asm_file, "-o", bin_file]
        print(f"[DEBUG] Running assembler: {' '.join(assemble_cmd)}")
        try:
            result = subprocess.run(assemble_cmd, capture_output=True, text=True, cwd=current_dir, timeout=10)
        except subprocess.TimeoutExpired:
            print(f"[DEBUG] Assembler timed out!")
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return {"output": "Assembly process timed out!"}
        print(f"[DEBUG] Assembler return code: {result.returncode}")
        print(f"[DEBUG] Assembler stdout: {result.stdout}")
        print(f"[DEBUG] Assembler stderr: {result.stderr}")

        output = ""
        if result.returncode != 0:
            output += f"Assembly failed.\nStderr:\n{result.stderr}\nStdout:\n{result.stdout}"
            if os.path.exists(asm_file):
                os.remove(asm_file)
            print(f"[DEBUG] Assembly failed, returning output.")
            return {"output": output}

        # Simulate
        simulator_path = os.path.join(current_dir, "ZX16_System_Simulator")
        sim_cmd = [simulator_path, bin_file]
        print(f"[DEBUG] Running simulator: {' '.join(sim_cmd)}")
        try:
            result = subprocess.run(sim_cmd, capture_output=True, text=True, cwd=current_dir, timeout=10)
        except subprocess.TimeoutExpired:
            print(f"[DEBUG] Simulator timed out!")
            if os.path.exists(asm_file):
                os.remove(asm_file)
            if os.path.exists(bin_file):
                os.remove(bin_file)
            return {"output": "Simulation process timed out!"}
        print(f"[DEBUG] Simulator return code: {result.returncode}")
        print(f"[DEBUG] Simulator stdout: {result.stdout}")
        print(f"[DEBUG] Simulator stderr: {result.stderr}")

        if os.path.exists(asm_file):
            os.remove(asm_file)
        if os.path.exists(bin_file):
            os.remove(bin_file)
        print(f"[DEBUG] Temp files cleaned up.")

        if result.returncode != 0:
            output += f"Simulation failed.\nStderr:\n{result.stderr}\nStdout:\n{result.stdout}"
            return {"output": output, "registers": None, "memory": None}
        else:
            output += f"Simulation succeeded.\nStdout:\n{result.stdout}"
            if result.stderr:
                output += f"\nStderr:\n{result.stderr}"

        # --- Parse register and memory dump from output ---
        registers = {}
        memory = []
        lines = result.stdout.splitlines()
        reg_section = False
        mem_section = False
        for line in lines:
            if line.strip().startswith("t0 ="):
                reg_section = True
            if reg_section and line.strip() == '':
                reg_section = False
            if reg_section:
                parts = line.strip().split('=')
                if len(parts) == 2:
                    reg, val = parts
                    registers[reg.strip()] = int(val.strip())
            if line.strip().startswith("Memory Dump"):
                mem_section = True
                continue
            if mem_section:
                if line.strip() == '':
                    mem_section = False
                    continue
                if ':' in line:
                    addr, bytestr = line.split(':', 1)
                    bytes_list = [b for b in bytestr.strip().split(' ') if b]
                    memory.append({"address": addr.strip(), "bytes": bytes_list})

        return {"output": output, "registers": registers, "memory": memory}
    except Exception as e:
        return JSONResponse(content={"error": f"Server error: {str(e)}"}, status_code=500)
