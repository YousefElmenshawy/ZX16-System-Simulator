FROM python:3.11-slim

WORKDIR /app

# Install dependencies: compiler + SFML + virtual display
RUN apt-get update && apt-get install -y \
    g++ \
    libsfml-dev \
    xvfb \
    x11vnc \
    && rm -rf /var/lib/apt/lists/*

# Install Python requirements
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy your project
COPY . .

# Build your simulator
RUN g++ -O2 -std=c++17 -o app/Backend/ZX16_System_Simulator Src/*.cpp \
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# Make it executable
RUN chmod +x app/Backend/ZX16_System_Simulator

# Start Xvfb, VNC, and backend
CMD Xvfb :0 -screen 0 1024x768x24 & \
    x11vnc -display :0 -nopw -listen 0.0.0.0 -xkb -forever -shared & \
    uvicorn app.Backend.main:app --host 0.0.0.0 --port ${PORT:-8080}
