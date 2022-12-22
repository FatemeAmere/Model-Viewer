#version 460 core

out vec4 FragColor;

in vec3 cubemapDir;
in vec3 normal;
in vec3 posForColoring;
in vec2 texCoord;
in vec3 cubemapDirForObject;

uniform samplerCube cubemap;

void main(){
	FragColor = texture(cubemap, cubemapDirForObject);
	//FragColor = vec4(cubemapDirForObject,1);
}
