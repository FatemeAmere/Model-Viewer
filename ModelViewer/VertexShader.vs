#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 tex;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 m;
uniform mat4 mvN;

out vec3 normal;
out vec3 posForColoring;
out vec2 texCoord;

out vec3 cubemapDir;
out vec3 pos_CubeMap;
out vec3 norm_CubeMap;

void main(){
	gl_Position = mvp * vec4(pos,1);
	posForColoring = (mv * vec4(pos,1)).xyz;
	normal = mat3(mvN) * norm;
	texCoord = tex;

	cubemapDir = pos;
	pos_CubeMap = (m*vec4(pos,1)).xyz;
	norm_CubeMap = transpose(inverse(mat3(m)))*norm;

}