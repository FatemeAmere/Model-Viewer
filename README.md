# Model-Viewer
A 3D obj model viewer using OpenGL and C++
The programs takes the name of the obj file as an argument and loads the model. (The input model has to be an obj file)
It works with models with multiple materials.
Currently, only these parameters of the material is used to render the model: Ka, Kd, Ks, Ns, map_Ka, map_Kd, map_Ks.

Hold left mouse button and drag to rotate the model.
Hold down LCTRL + left mouse button and drag to rotate the light source around the model.
Hold right mouse button and drag to change the camera distance.

Here is a demo with the famouse Utah teapot:
![Alt Text](demo.gif)