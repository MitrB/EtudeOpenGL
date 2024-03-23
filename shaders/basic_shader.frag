#version 330 core
out vec4 FragColor;  

in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
  
void main()
{
    float ambient_strength = 0.2;
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    vec3 result = (ambient_strength + diffuse) * object_color;
    FragColor = vec4(result, 1.0);
}