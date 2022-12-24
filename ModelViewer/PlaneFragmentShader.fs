#version 460 core

out vec4 FragColor;

in vec3 normal;
in vec3 posForColoring;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float specularExponent;

uniform vec3 lightPosition;

uniform bool useCubeMap;
uniform samplerCube cubemap;

uniform mat4 cubeMapOrientation;
in vec3 pos_CubeMap;
in vec3 norm_CubeMap;

void main(){
	vec3 I = vec3(1,1,1);
	vec3 ambient = vec3(0.1f,0.1f,0.1f);

	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPosition - posForColoring);
	vec3 view = normalize(-posForColoring);
	vec3 halfVec = normalize(lightDir+view);

	vec3 diffuse = max(0,dot(norm,lightDir)) * kd;
	vec3 specular = pow(max(0,dot(halfVec, norm)),specularExponent) * ks;
	vec3 blinn = I*(diffuse + specular) + ambient*ka;

	//cubemap
	vec3 v = normalize(pos_CubeMap - vec3(0,0,30));
    vec3 R = reflect(v, normalize(norm_CubeMap));
    vec3 reflection = useCubeMap ? texture(cubemap, inverse(mat3(cubeMapOrientation)) * R).rgb : vec3(0,0,0);

	FragColor = vec4(blinn + reflection * ks, 1.0f);
	//FragColor = vec4(1,1,1,1);
}