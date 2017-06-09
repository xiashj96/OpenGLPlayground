#version 430 core
// this shader implements phong shading with Ambient/Diffuse/Specular lighting model

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    float intensity; // assume the light is white
    // attenuation parameters
    float constant;
    float linear;
    float quadratic;
};

in vec3 normalView;
in vec3 fragPosView;
uniform Light light;
uniform Material material;
uniform mat4 ciViewMatrix;
out vec4 oColor;

void main()
{
    // perform lighting calculation in view space
    vec3 light_pos_view = vec3(ciViewMatrix*vec4(light.position,1));
    vec3 light_dir = normalize(light_pos_view - fragPosView);
    float light_dis = distance(light_pos_view, fragPosView);
    vec3 view_dir = normalize(-fragPosView);
    vec3 reflect_dir = reflect(-light_dir, normalView);

    // diffuse
    float attenuation = 1/(light.constant+light.linear*light_dis+light.quadratic*light_dis*light_dis);
    float diff_strength = max(dot(normalView, light_dir), 0.0) * attenuation * light.intensity;
    // specular
    float specular_strength = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess) * attenuation * light.intensity;
    oColor = vec4(
        clamp(material.ambient + diff_strength * material.diffuse + specular_strength * material.specular, 0, 1),
        1 );
}