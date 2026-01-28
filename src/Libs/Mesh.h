#ifndef MESH____H
#define MESH____H

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <GL/glew.h>


// Interleaved vertex: position (3), normal (3), uv (2)
struct VertexPNU 
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 uv{};
};

class Mesh 
{
    public:
        Mesh();
        ~Mesh();

        // Create from client-provided interleaved vertex array (8 floats per vertex: P(3) N(3) UV(2))
        // Common in many tutorials; kept for compatibility.
        void CreateMesh(const GLfloat* vertices,
                const unsigned int* indices,
                unsigned int numOfFloats,
                unsigned int numOfIndices);

        // Create from interleaved vector of VertexPNU (preferred)
        void CreateMesh(const std::vector<VertexPNU>& vertices,
        const std::vector<unsigned int>& indices);

        // Load from .obj (supports v, vt, vn; faces in v/vt/vn, v//vn, v/vt, or v)
        // Handles negative indices; triangulates n-gons via fan.
        // flipV = true to flip the V coordinate (uv.y = 1 - uv.y)
        bool CreateMeshFromOBJ(const std::string& path, bool flipV = false);

        void RenderMesh() const;
        void ClearMesh();

        // Accessors
        inline GLsizei GetIndexCount() const { return indexCount; }
        inline GLuint GetVAO() const { return VAO; }

    private:
        // GL resources
        GLuint VAO{0};
        GLuint VBO{0};
        GLuint IBO{0};

        GLsizei indexCount{0};

        // Internal helper used by OBJ loader
        void uploadInterleaved_(const std::vector<VertexPNU>& vertices,
        const std::vector<unsigned int>& indices);
};

#endif