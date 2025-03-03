#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Bone Transforms
#define MAX_BONES 100
uniform mat4 bones[MAX_BONES];

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;
in ivec4 aBoneId; // Bone Ids
in vec4 aBoneWeight; // Bone Weights

out vec4 color;
out vec2 texCoord0;
out vec4 position;
out vec3 normal;
out vec3 texCoordCubeMap;

uniform mat4 matrixShadow;
out vec4 shadowCoord;

// Light declarations
struct AMBIENT 
{
    vec3 color;
};

uniform AMBIENT lightAmbient1, lightAmbient2;

vec4 AmbientLight(AMBIENT light) 
{
    // Calculate Ambient Light
    return vec4(materialAmbient * light.color, 1);
}

struct DIRECTIONAL
{
    vec3 direction;
    vec3 diffuse;
};

uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
    // Calculate Directional Light
    vec4 color = vec4(0, 0, 0, 0);
    vec3 L = normalize(mat3(matrixView) * light.direction);
    float NdotL = dot(normal, L);
    color += vec4(materialDiffuse * light.diffuse, 1) * max(NdotL, 0);
    return color;
}

void main(void) 
{
    mat4 matrixBone;
    if (aBoneWeight[0] == 0.0)
        matrixBone = mat4(1);
    else
        matrixBone = (bones[aBoneId[0]] * aBoneWeight[0] + 
                      bones[aBoneId[1]] * aBoneWeight[1] + 
                      bones[aBoneId[2]] * aBoneWeight[2] + 
                      bones[aBoneId[3]] * aBoneWeight[3]);

    // calculate position
    position = matrixModelView * matrixBone * vec4(aVertex, 1.0);
    gl_Position = matrixProjection * position;

    normal = normalize(mat3(matrixModelView) * mat3(matrixBone) * aNormal);
    texCoord0 = aTexCoord;

    mat4 matrixModel = inverse(matrixView) * matrixModelView;
    shadowCoord = matrixShadow * matrixModel * vec4(aVertex + aNormal * 0.1, 1);
    texCoordCubeMap = inverse(mat3(matrixView)) * mix(reflect(position.xyz, normal.xyz), normal.xyz, 0.2);

    // calculate light
    color = vec4(0, 0, 0, 1);
    color += AmbientLight(lightAmbient1);
    color += AmbientLight(lightAmbient2);
    color += DirectionalLight(lightDir);
}
