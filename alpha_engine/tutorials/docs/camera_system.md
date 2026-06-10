# Camera System

## Basics of camera movement

In computer graphics, the idea of a "camera" is simply a matrix that is applied on top of the transform that you set before rendering a mesh. 
In this section, we are go through the basics of implementing a "camera".

It illustrate this, let's use our previous [sprite rendering example](./rendering_sprites.md/). 

[When we calculated its transformation matrix](./rendering_sprites.md#calculating-the-transformation-matrix), we formed the matrix transformation of our sprite using TRS:

$$
T_{sprite} =
\begin{bmatrix}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
\cos(r) & -\sin(r) & 0 \\
\sin(r) & \cos(r) & 0 \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
s_x & 0 & 0 \\
0 & s_y & 0 \\
0 & 0 & 1
\end{bmatrix}
$$


In computer graphics, we don't have a "camera" object per se. 
What we have are transforms to concatenate together for each sprite.
We can use this to simulate the effect of a "camera".

Let's start with camera movement.
If we wanted to "move our camera to the right", what we will essentially be doing is to move our sprites to the left.
Likewise, if we wanted to "move our camera upwards", we will move our sprites downwards. 
Essentially, we will translate our sprites the negative of the camera's position.

As an example, if our sprite is at `x = 100` and our "camera" is at `x = 100`, what we should see is that our sprite is rendered *as if* it's at `x = 0`.

We do this for every sprite that we want to be affected by our so-called "camera".

Therefore, the idea of a implementing camera movement is simply to concatenate our sprite's transform with a translation matrix that moves our object by the negative camera position. 


$$
T_{sprite} =
\begin{bmatrix}
1 & 0 & -c_x \\
0 & 1 & -c_y \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
1 & 0 & t_x \\
0 & 1 & t_y \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
\cos(r) & -\sin(r) & 0 \\
\sin(r) & \cos(r) & 0 \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
s_x & 0 & 0 \\
0 & s_y & 0 \\
0 & 0 & 1
\end{bmatrix}
$$

where $c_x$ is the camera's X position and $c_y$ is the camera's Y position.

## Camera movement with Alpha Engine

Alpha Engine provides a few functions that will simulate camera movements all meshes drawn by `AEGfxMeshDraw()`:

* AEGfxSetCamPosition()
* AEGfxGetCamPosition().

## Exercise

Try to write your own camera movement system without using Alpha Engine's camera functions!
Can you come up with a camera system that does zooms and rotations?

