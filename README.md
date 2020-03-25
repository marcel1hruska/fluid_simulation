# Fluid simulation

![Waterfall upon forest](/images/screen1.png)

WIP!

Simulation of water surface using virtual pipeline model in OpenGl, done from scratch. The implementation recomputes water outflows in each frame.

# How to run

The project is currently buildable as it is only on Windows via cmake for x86 Release. You may download released version (again, Windows only) [here](https://github.com/marcel1hruska/fluid_simulation/releases/tag/0.5).

# Usage

User can move around using WASD keys and mouse to rotate the camera.

You may exit the application any time by pressing ESC.

To start the simulation, press SPACE. While the simulation is running, it is possible to interact with the water in multiple ways by holding the right mouse button.

Changing the settings pauses the simulation.

The water settings determine the initial position of water.

The terrain settings determine the shape of the terrain.

The speed may slow down the water flow.

It is possible to interact with the water in multiple ways:
- Manual: manually adds water where the cursor aims
- Rain: simulates water droplets on terrain/water surface
- Waterfall: adds lot of water from specific corner
- Touch: water surface deformation on touch
- Object: puts object in the water, may be move to see waves (WIP! Doesn't work properly)

![Rain drops on plain water surface](/images/screen2.png)
