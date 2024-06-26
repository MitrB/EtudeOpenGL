#version 330 core
layout (location = 0) in vec3 aPos;  
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 aTexCoords;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    frag_pos = vec3(model * vec4(aPos, 1.0));
    // inverse calculation is very heavy for the gpu, calc this on the cpu
    frag_normal = mat3(transpose(inverse(model))) * normal;
    frag_texcoords = aTexCoords;
}