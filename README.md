a tool to plot out a path for robots to traverse using the power of *math*.

# plot

This is a graphical tool to lay out paths that are made in a way that is easy to calculate for a robot traversing it. This is because paths are made entirely out of a series of arcs (circles).

# Theory

Math pretty much just copied from [here](https://www.cs.columbia.edu/~allen/F19/NOTES/icckinematics.pdf).

```
w ( R + l / 2 ) = Vr
w ( R - l / 2 ) = Vl
```

```
R = l / 2 ((Vl + Vr) / (Vr - Vl))
```

```
w = (Vr - Vl) / l
```

`w` is the rate of rotation,

`l ` is the distance between the centers of the two wheels,

`R` is the signed distance from the ICC to the midpoint between the wheels,

`Vr & Vl` are the right and left wheel velocities,

`IIC` is the center of rotation.



`IIC` is stored in the path so we don’t need to calculate it on the robot’s side. The whole point of this application is to calculate those `ICC` points.

# Build Requirements

Only tested on windows so far with the following setup:

- Clang compiler for Windows (https://github.com/llvm/llvm-project)
- Visual Studio 2019 (https://visualstudio.microsoft.com/downloads/)

# Status

Basic math implemented for calculating the paths and that’s about it.

# Additional Resources

- [Introduction to Robotics #3: Forward and Inverse Kinematics](http://correll.cs.colorado.edu/?p=896)
- [Holy Grail of Differential Drive Kinematics](https://www.cs.columbia.edu/~allen/F17/NOTES/icckinematics.pdf)
- [More Kinds of Kinematics](https://course.ccs.neu.edu/cs4610/L3/L3.html)
- [Includes Mecanum Kinematics](http://robotsforroboticists.com/drive-kinematics/)

