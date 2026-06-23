# Fluid sim

A work-in-progress real-time fluid simulation built in C++ and OpenGL, using
Eulerian (grid-based) techniques. Instead of tracking moving particles, the fluid
lives on a fixed 3D voxel grid where every cell stores field quantities — velocity,
pressure, and density. The solver advances the incompressible Navier–Stokes
equations across that grid, and the result is then visualized by building a surface
mesh from the density field with marching cubes.

## How it works

The grid is updated in place each step:

- **Advection** carries the field quantities along the velocity field, using
  semi-Lagrangian backtracing so it stays stable even at large time steps.
- **Forces** such as gravity and buoyancy are applied to the velocity field.
- **Pressure projection** solves a Poisson equation for pressure and subtracts its
  gradient, making the velocity field divergence-free so the fluid stays
  incompressible. The solve is iterative (e.g. Jacobi / Gauss–Seidel).
- **Boundary conditions** stop flow from entering solid cells and handle the edges
  of the domain.

To render, the scalar field is converted into a triangle mesh with marching cubes
and drawn through the OpenGL pipeline, turning the discrete voxel data into a
continuous surface.

## Plan

The simulation is built in two stages that are kept fully decoupled:

1. **CPU first.** A direct CPU implementation of the grid solver acts as the
   reference — easy to step through, debug, and validate visually.
2. **GPU second.** The same algorithm is then ported to OpenGL compute shaders that
   operate on the grid on the device.

Rather than branching between them at runtime, the backend is selected at **build
time through templates**. Both implementations satisfy the same compile-time
interface (initialise, step, expose the grid), and the engine is instantiated against
whichever one is chosen. This keeps the two paths independent — GPU work can't
destabilise the CPU path — and removes any virtual-dispatch cost from the solver
loop.

## Dependencies

Everything is vendored under `vendor/` and built from source through CMake, so the
only things you need installed are a C++17 compiler and an OpenGL-capable driver.

- **OpenGL** — rendering and GPU compute
- **GLFW** — windowing and input
- **GLM** — vector and matrix math
- **Glad** — OpenGL function loader
- **Dear ImGui** — in-app controls for tuning parameters and reading timings

## Build

```bash
cmake -B build
cmake --build build --config Release
```

Run the `FluidSim` target from the build directory. Shaders are copied next to the
executable as a post-build step, and the shader folder is also passed to the program
via the `SHADER_DIR` compile definition, so they load correctly whether you launch
from the build folder or an IDE.
