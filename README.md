# Simple 3D Game

A lightweight 3D prototype inspired by *Temple Run*, built using **OpenGL**, **GLFW**, and **Assimp**.  
The player continuously runs forward while gravity and collisions keep the character grounded.  
Collectible spheres (coins) spawn randomly above the floor — touching them makes them disappear.

---

## Assets
- **Player model:** [Temple Run Guy Dangerous (Sketchfab)](https://sketchfab.com/3d-models/temple-run-guy-dangerous-9d392ef3d02a4dd28ce9e5795161caab)
- **Floor texture:** `textures/marble.jpg`
- **Shaders:** `shaders/templerun.vert`, `shaders/templerun.frag`

---

## How to Play
- The player runs forward automatically.
- Press **A** or **D** to rotate 90° left or right.
- Touch colored spheres to collect them.
- Press **ESC** to quit.

---

## Description
This project demonstrates:
- Basic model loading (FBX via Assimp)
- Lighting and texture mapping
- Camera that follows the player
- Simple physics (gravity, floor collision)
- Collision detection (player–sphere)
- Procedural generation of collectibles

---

## Known Bugs
- The player model flips or faces the wrong direction after rotation (orientation mismatch with camera).
- No animation yet; the player appears static while moving.

---

## Future Work
- Add character animations (run, jump, slide).
- Build a longer map or procedural level generation.
- Implement jumping, sliding, and obstacles.
- Fix model orientation alignment.
- Add UI (score counter, start/restart menu).

- [![Temple Run Demo]](https://github.com/yourusername/TempleRun/blob/main/demo/s3d.mkv)

