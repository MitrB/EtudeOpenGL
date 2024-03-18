#version 330 core
out vec4 FragColor;  

// in vec3 ourColor;
// in vec2 TexCoord;

// uniform sampler2D texture0;
// uniform sampler2D texture1;
// uniform float mix_value;
  
void main()
{
    // FragColor = mix(texture(texture0, TexCoord), texture(texture1, vec2(-TexCoord.x, TexCoord.y)), mix_value);
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}