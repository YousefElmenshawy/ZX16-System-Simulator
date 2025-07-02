from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse
import subprocess
import uuid
import os
from fastapi.middleware.cors import CORSMiddleware
import logging

# Get the directory where main.py is located
BACKEND_DIR = os.path.dirname(os.path.abspath(__file__))

# Configuration
ASSEMBLER_TIMEOUT = 10  # seconds
SIMULATOR_TIMEOUT = 10  # seconds

app = FastAPI()

# 👇 Add this to allow requests from any frontend origin (for development)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Or ["http://localhost:5500"] for specific origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Configure logging
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

@app.post("/simulate")
async def simulate_code(request: Request):
    logging.debug("/simulate endpoint called")
    try:
        data = await request.json()
        code = data.get("code", "")
        logging.debug(f"Received code: {code[:40]}...")

        uid = str(uuid.uuid4())
        asm_file = os.path.join(BACKEND_DIR, f"temp_{uid}.s")
        bin_file = os.path.join(BACKEND_DIR, f"temp_{uid}.bin")

        # Save the assembly code
        try:
            with open(asm_file, "w") as f:
                f.write(code)
            logging.debug(f"Written assembly code to {asm_file}")
        except Exception as e:
            logging.error(f"Failed to write assembly file: {e}")
            return JSONResponse(content={"error": f"Failed to write assembly file: {str(e)}"}, status_code=500)

        # Assemble
        assemble_cmd = ["python3", os.path.join(BACKEND_DIR, "zx16asm.py"), asm_file, "-o", bin_file]
        logging.debug(f"Running assembler: {assemble_cmd}")
        try:
            result = subprocess.run(assemble_cmd, capture_output=True, text=True, timeout=ASSEMBLER_TIMEOUT, cwd=BACKEND_DIR)
            logging.debug(f"Assembler process completed with return code: {result.returncode}")
        except FileNotFoundError as e:
            logging.error(f"Assembler not found: {e}")
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return JSONResponse(content={"error": f"Assembler script not found: {str(e)}"}, status_code=500)
        except subprocess.TimeoutExpired as e:
            logging.error(f"Assembler timed out after {ASSEMBLER_TIMEOUT} seconds")
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return JSONResponse(content={"error": f"Assembly process timed out after {ASSEMBLER_TIMEOUT} seconds. Your code might have an infinite loop or be too complex."}, status_code=500)

        if result.stdout:
            logging.debug(f"Assembler stdout: {result.stdout}")
        if result.stderr:
            logging.debug(f"Assembler stderr: {result.stderr}")

        if result.returncode != 0:
            error_msg = result.stderr or "Unknown assembler error"
            if os.path.exists(asm_file):
                os.remove(asm_file)
            return JSONResponse(content={"error": error_msg}, status_code=400)

        # Simulate
        sim_cmd = [os.path.join(BACKEND_DIR, "ZX16_System_Simulator"), bin_file]
        logging.debug(f"Running simulator: {sim_cmd}")
        try:
            result = subprocess.run(sim_cmd, capture_output=True, text=True, timeout=SIMULATOR_TIMEOUT, cwd=BACKEND_DIR)
        except FileNotFoundError as e:
            logging.error(f"Simulator not found: {e}")
            for f in [asm_file, bin_file]:
                if os.path.exists(f):
                    os.remove(f)
            return JSONResponse(content={"error": f"Simulator binary not found: {str(e)}"}, status_code=500)
        except subprocess.TimeoutExpired as e:
            logging.error(f"Simulator timed out after {SIMULATOR_TIMEOUT} seconds")
            for f in [asm_file, bin_file]:
                if os.path.exists(f):
                    os.remove(f)
            return JSONResponse(content={"error": f"Simulation timed out after {SIMULATOR_TIMEOUT} seconds. Your code might have an infinite loop."}, status_code=500)

        # Clean up temp files
        for f in [asm_file, bin_file]:
            if os.path.exists(f):
                os.remove(f)

        return {"output": result.stdout, "simulator_error": result.stderr}
    except Exception as e:
        logging.error(f"Exception in /simulate: {str(e)}")
        return JSONResponse(content={"error": f"Internal server error: {str(e)}"}, status_code=500)
