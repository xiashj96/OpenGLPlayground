#version 430 core
// this shader implements phong shading with Ambient/Diffuse/Specular lighting model
uniform mat4 ciModelView;
uniform mat4 ciProjectionMatrix;
uniform mat3 ciModelViewInverseTranspose; // normal matrix
in vec4 ciPosition;
in vec3 ciNormal;
out vec3 normalView;
out vec3 fragPosView;

void main()
{
    gl_Position = ciProjectionMatrix * ciModelView * ciPosition;
    normalView = normalize(vec3(ciModelViewInverseTranspose * ciNormal));
    fragPosView = vec3(ciModelView * ciPosition);
}