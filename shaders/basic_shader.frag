#version 330 core
out vec4 FragColor;  

in vec3 frag_pos;
in vec3 frag_normal;

uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform vec3 view_pos;
uniform float ambient_strength;
uniform float specular_strength;
uniform float shinyness;
  
void main()
{
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light_pos - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflected_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflected_dir), 0.0), shinyness);
    vec3 specular = specular_strength * spec * light_color;

    vec3 result = (ambient + diffuse + specular) * object_color;
    FragColor = vec4(result, 1.0);
}