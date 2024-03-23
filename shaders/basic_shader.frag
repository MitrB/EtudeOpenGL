#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

out vec4 FragColor;

in vec3 frag_pos;
in vec3 frag_normal;

uniform Material material;
uniform Light light;
uniform vec3 object_color;
uniform vec3 light_color;
uniform vec3 view_pos;
  
void main()
{
    vec3 ambient = light.ambient * material.ambient;

    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflected_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, reflected_dir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}