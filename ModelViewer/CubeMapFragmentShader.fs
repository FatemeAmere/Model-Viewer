#version 460 core

out vec4 FragColor;

in vec3 cubemapDir;

uniform samplerCube cubemap;


void main(){
	FragColor = texture(cubemap, cubemapDir);
}
