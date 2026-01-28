
#include <string>
#include <vector>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "Libs/Mesh.h"
#include "Libs/stb_image.h"

Shader *PrepareShader(const char *vShader, const char *fShader);
Mesh *PreparePlane();
Mesh *PrepareModelMesh(std::string path);
GLuint PrepareTexture(std::string path);
unsigned int PrepareObject(std::string meshPath, std::string texturePath);
unsigned int PrepareObjectFromMesh(Mesh *mesh, std::string texturePath);

// Structure to hold meshs, textures and shaders
struct Object
{
    Mesh *mesh;
    GLuint texture;
    Shader *shader;

    // draw function ptr callback
    void (*drawPrt)(Object *, glm::mat4, glm::mat4, glm::vec3) = nullptr;

    void UseShader();
    void Draw(glm::mat4 view, glm::mat4 projection, glm::vec3 position);
};

extern std::vector<Object> objects;