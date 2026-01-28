#include <string>
#include <vector>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "Libs/Mesh.h"
#include "Libs/stb_image.h"
#include "utils.h"

std::vector<Object> objects;

void Object::UseShader()
{
    this->shader->UseShader();
}

void Object::Draw(glm::mat4 view, glm::mat4 projection, glm::vec3 position)
{
    if (this->drawPrt != nullptr)
    {
        this->drawPrt(this, view, projection, position);
        return;
    }
    std::cout << "No draw function ptr set!" << std::endl;
}

Shader *PrepareShader(const char *vShader, const char *fShader)
{
    std::cout << "Preparing shader from files: " << vShader << " and " << fShader << std::endl;
    Shader *shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    return shader1;
}

// prepare plane mesh return mesh idx
Mesh *PreparePlane()
{

    GLfloat vertices[] =
        {
            // pos //TexCoord
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0, -1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    unsigned int indices[] =
        {
            0, 1, 2,
            2, 3, 0};
    Mesh *obj = new Mesh();
    obj->CreateMesh(vertices, indices, 20, 6);
    return obj;
}

// prepare model from obj file, return mesh
Mesh *PrepareModelMesh(std::string path)
{
    Mesh *obj1 = new Mesh();
    bool loaded = obj1->CreateMeshFromOBJ(path);
    if (loaded)
    {
        return obj1;
    }
    std::cout << "Failed to load model" << std::endl;
}

// prepare texture from file return texture id
GLuint PrepareTexture(std::string path)
{
    stbi_set_flip_vertically_on_load(true);
    unsigned int texture1;
    int width1, height1, nrChannels1;
    unsigned char *data1 = stbi_load("Textures/Final.png", &width1, &height1, &nrChannels1, 0);

    if (!data1)
    {
        std::cerr << "Failed to load texture: Textures/Final.png" << std::endl;
    }
    else
    {
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format = (nrChannels1 == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width1, height1, 0, format, GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    return texture1;
}

// prepare object with mesh, texture and shader return object idx
unsigned int PrepareObject(std::string meshPath, std::string texturePath)
{
    Object obj;
    obj.mesh = PrepareModelMesh(meshPath);
    obj.texture = PrepareTexture(texturePath);
    objects.push_back(obj);
    return objects.size() - 1;
}

unsigned int PrepareObjectFromMesh(Mesh *mesh, std::string texturePath)
{
    Object obj;
    obj.mesh = mesh;
    if (texturePath != "")
    {
        obj.texture = PrepareTexture(texturePath);
    }
    objects.push_back(obj);
    return objects.size() - 1;
}