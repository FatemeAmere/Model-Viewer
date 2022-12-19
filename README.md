# Model-Viewer
A 3D obj model viewer using OpenGL and C++
The programs takes the name of the **obj** file as an argument and loads the model.
It works with models with multiple materials.
Currently, only these parameters of the material is used to render the model: Ka, Kd, Ks, Ns, map_Ka, map_Kd, map_Ks.

Hold left mouse button and drag to rotate the model.
Hold down LCTRL + left mouse button and drag to rotate the light source around the model.
Hold right mouse button and drag to change the camera distance.

[stb_image.h] (https://github.com/nothings/stb/blob/master/stb_image.h) was used for loading textures.
[cyTriMesh.h] (https://github.com/cemyuksel/cyCodeBase/blob/master/cyTriMesh.h) was used for reading model data.
The project uses OpenGL 4.6.

Here is a demo with the famouse Utah teapot:
![Alt Text](demo.gif)
