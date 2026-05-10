# CPU Scheduling Simulator

A graphical desktop simulator for visualizing and comparing CPU scheduling algorithms. Built with C++ and raylib, this project lets users enter process arrival times, burst times, priorities, and quantum values, then watch scheduling behavior and Gantt chart execution.

## Features

- Visual simulation of CPU scheduling algorithms
- Interactive process input with arrival time, burst time, and priority
- Gantt chart visualization of execution order and idle time
- Built-in comparison mode for multiple scheduling strategies
- Audio and image support for enhanced UI experience

## Supported Algorithms

- FCFS (First-Come, First-Served)
- SJF (Shortest Job First, non-preemptive)
- Round Robin
- SRTF (Shortest Remaining Time First)
- LRTF (Longest Remaining Time First)
- Priority Scheduling (preemptive)
- Priority Scheduling (non-preemptive)
- MLFQ (Multi-Level Feedback Queue)
- Compare All (run and compare multiple algorithms)

## Requirements

- Windows or compatible desktop environment
- C++ compiler with C++17 support
- `mingw32-make` or GNU Make
- raylib library installed and accessible via `RAYLIB_PATH`
- `audios/` and `images/` folders are optional but used if available

## Build Instructions

1. Open a terminal in the project folder.
2. Run the debug build task or use make directly:

```powershell
C:/raylib/w64devkit/bin/mingw32-make.exe RAYLIB_PATH=C:/raylib/raylib PROJECT_NAME=main OBJS=*.cpp BUILD_MODE=DEBUG
```

3. To build release mode, change `BUILD_MODE=RELEASE`.

## Run Instructions

- After building, run the generated executable `main.exe` from the project folder.
- The program opens a window where you can:
  - enter the number of processes
  - set arrival time, burst time, and priority
  - choose a scheduling algorithm
  - view the generated Gantt chart and metrics

## Project Structure

- `main.cpp` — application entrypoint and UI logic
- `simulator.cpp`, `simulator.h` — scheduler execution and result handling
- `ranker.cpp`, `ranker.h` — algorithm ranking and comparison logic
- `process.h` — process structure definitions
- `Algos/` — individual algorithm implementations
- `Headers/` — shared algorithm headers
- `audios/` — audio files for UI sound effects
- `images/` — background and asset images

## Notes

- If raylib assets like `audios/a2.wav` or `images/bg.jpg` are missing, the simulator still runs without them.
- Use a consistent raylib installation path in the makefile or build command.

## License

This project is provided as-is. See `LICENSE.txt` for more details.
