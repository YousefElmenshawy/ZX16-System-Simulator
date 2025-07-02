from fastapi import FastAPI, Request
from fastapi.responses import JSONResponse
import subprocess
import uuid
import os
from fastapi.middleware.cors import CORSMiddleware



app = FastAPI()

# 👇 Add this to allow requests from any frontend origin (for development)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Or ["http://localhost:5500"] for specific origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/simulate")
async def simulate_code(request: Request):
    data = await request.json()
    code = data["code"]

    uid = str(uuid.uuid4())
    asm_file = f"temp_{uid}.s"
    bin_file = f"temp_{uid}.bin"

    with open(asm_file, "w") as f:
        f.write(code)

    # Assemble
    assemble_cmd = ["python", "./zx16asm.py", asm_file, "-o", bin_file]
    result = subprocess.run(assemble_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        return JSONResponse(content={"error": result.stderr}, status_code=400)

    # Simulate
    sim_cmd = ["./ZX16_System_Simulator.exe", bin_file]
    result = subprocess.run(sim_cmd, capture_output=True, text=True)

    # Clean up temp files
    os.remove(asm_file)
    os.remove(bin_file)
    print("==== SIMULATOR STDOUT ====")
    print(result.stdout)
    print("==== SIMULATOR STDERR ====")
    print(result.stderr)

    return {"output": result.stdout}
