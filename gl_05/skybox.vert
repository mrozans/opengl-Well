#version 330 core

layout (location = 0) in vec3 position;

out vec3 TexCoord;
  
uniform mat4 view;
uniform mat4 projection;

void main()
{
	vec4 pos = projection * view *  vec4(position, 1.0f);
	gl_Position = pos.xyww;
    TexCoord = position;
} 