# CatchingDrone
## RT simulation (3D) of a PD controlled quadcopter
The purpose of the assigned work was to design a program to control a virtual drone that catches a ball thrown by the user. In particular it was required the interaction with a 3D graphic simulation engine (Unreal Engine) to display a virtual environment where virtual drone operates. Drone modeling and control has to be implemented in Linux as real-time tasks.

### Video
[![A short video of the working project](https://img.youtube.com/vi/c7JfwRTDGME/0.jpg)](http://www.youtube.com/watch?v=c7JfwRTDGME)

### UnrealEngine executables (Mac OS & Win) and project (Win)
https://drive.google.com/open?id=0B25d6vf2iGnaN0JudnJvTmhBNTg

### Read more
Read the report for the entire project development and information

### How to
- Clone the repository.
- Change IP address of the machine who will host the UE4 executable, inside src/main.c
- Compile (make provided) sources inside src folder (please check that the Allegro library is installed and correctly linked and loaded).
- Open UE4 executable
- Execute (with sudo privileges) the main program


