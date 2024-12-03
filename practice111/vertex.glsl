#version 330 core

//--- in_Position: attribute index 0 
//--- in_Color: attribute index 1

layout (location = 0) in vec3 vPos; //--- 위치 변수: attribute position 0
layout (location = 1) in vec3 vNormal; //--- 컬러 변수: attribute position 1

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model = mat4(1.0f); // 모델링변환
uniform mat4 view = mat4(1.0f); // 뷰잉변환
uniform mat4 projection = mat4(1.0f); // 투영변환

void main(void) 
{
	gl_Position = projection * view * model * vec4(vPos, 1.0);
	FragPos = vec3(model * vec4(vPos, 1.0));
	Normal = vNormal;
}
