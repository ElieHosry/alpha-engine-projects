# Using Spritesheets

Alpha Engine does not provide a spritesheet animation system, but it does provide enough tools for you to implement one. 

## Calculating the UV

First, you must build a mesh that fits your spritesheet and the animation you want to do. 
Let's say we have a spritesheet like the one below:

![](res/spritesheet.png)

In this case, the spritesheet consists of 5 rows and 4 columns sprites.

In this example, we will use the [same 1x1 mesh that we defined](rendering_sprites.md#creating-a-1x1-mesh) before, but there is a problem: the UV coordinates for each vertex in that mesh encompasses the whole image.
This brings us to the next step: we need to calculate the UV coordinates such that it encompasses a sprite in the spritesheet.

!!! tip 
    
    If you need a refresher on what UV coordinates are, you can refer to <<sprite_mesh>>.

Let's calculate the UV such that we target the top-left sprite.
Since we have 5 rows and 4 columns in the spritesheet, a sprite's width is 1/4 of the spritesheet's width and its height is 1/5 of the spritesheet's height.   

Thus, given that:

* $v_{tl}$ is the top left vertex of the mesh.
* $v_{tr}$ is the top right vertex of the mesh.
* $v_{bl}$ is the bottom left vertex of the mesh.
* $v_{br}$ is the bottom left vertex of the mesh.
* $c$ is the number of columns in your spritesheet. 
* $r$ is the number of rows in your spritesheet. 

The formula for calculating the UVs for each vertex of our 1x1 sprite to target the top-left sprite is as follows:

$$ 
v_{tl} = \begin{bmatrix} 0 \\ 0 \end{bmatrix} \quad v_{tr} = \begin{bmatrix} \frac{1}{c} \\ 0 \end{bmatrix} \quad v_{bl} = \begin{bmatrix} 0 \\ \frac{1}{r} \end{bmatrix} \quad v_{br} = \begin{bmatrix} \frac{1}{c} \\ \frac{1}{r} \end{bmatrix}
$$

which gives us:

$$
v_{tl} = \begin{bmatrix} 0 \\ 0 \end{bmatrix} \quad v_{tr} = \begin{bmatrix} \frac{1}{4} \\ 0 \end{bmatrix} \quad v_{bl} = \begin{bmatrix} 0 \\ \frac{1}{5} \end{bmatrix} \quad v_{br} = \begin{bmatrix} \frac{1}{4} \\ \frac{1}{5} \end{bmatrix}
$$

The code to create the 1x1 mesh with the UV coordinates calculated above is therefore:


```c
AEGfxMeshStart();

AEGfxTriAdd(
  -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1/5.f, // bottom left
  0.5f, -0.5f, 0xFFFFFFFF, 1/4.f, 1/5.f, // bottom right 
  -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);  // top left

AEGfxTriAdd(
  0.5f, -0.5f, 0xFFFFFFFF, 1/4.f, 1/5.f, // bottom right 
  0.5f, 0.5f, 0xFFFFFFFF, 1/4.f, 0.0f,   // top right
  -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);  // bottom left

AEGfxVertexList * pMesh = AEGfxMeshEnd();
```

!!!warning 
    
    This means that for each spritesheet using a different set of UV coordinates, you will need create a seperate mesh! 

## UV Offset

Now that we have a mesh that can the top-left sprite of a spritesheet, how can we target the rest of the sprites?
The key lies in `AEGfxTextureSet()`. 
The 2nd and 3rd parameter allows you to pass in an offset that will modify the UV of our vertices. 

In the case of the spritesheet that we are using in this example, this means that:

* The first sprite's offset is $x = 0, y = 0$.
* The sprite on the first row, second column is $x = 1/4, y = 0$.
* The sprite on the second row, second column is $x= 1/4, y = 1/5$.
* And so on.

This gives us the ability to choose which section of the spritesheet we want to render, which subsequently allows us to implement spritesheet animation!
The code that performs spritesheet animation for this spritesheet can be found at `snippets/spritesheet.cpp`.
