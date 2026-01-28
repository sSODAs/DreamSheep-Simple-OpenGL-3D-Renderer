#include "Mesh.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <glm/geometric.hpp>

// Lock the attribute locations we’ll use everywhere
static constexpr GLuint ATTR_POS = 0;
static constexpr GLuint ATTR_UV  = 1;  // << UV at 1
static constexpr GLuint ATTR_NRM = 2;  // << normal at 2

// ----------------------------
// Mesh: lifecycle
// ----------------------------
Mesh::Mesh() {}

Mesh::~Mesh() { ClearMesh(); }

// ----------------------------
// Create from raw arrays (8 floats per vertex)
// ----------------------------
void Mesh::CreateMesh(const GLfloat* vertices,
                    const unsigned int* indices,
                    unsigned int numOfFloats,
                    unsigned int numOfIndices) 
{
    // Build vectors and forward to preferred path
    std::vector<VertexPNU> vtx;

    if (numOfFloats % 8 == 0) 
    {
        // Layout: P(3) N(3) UV(2)
        unsigned int numVerts = numOfFloats / 8;
        vtx.reserve(numVerts);
        for (unsigned int i = 0; i < numVerts; ++i) 
        {
            const GLfloat* base = vertices + i * 8;
            VertexPNU v{};
            v.position = { base[0], base[1], base[2] };
            v.normal   = { base[3], base[4], base[5] };
            v.uv       = { base[6], base[7] };
            vtx.push_back(v);
        }
    }
    else if (numOfFloats % 5 == 0) 
    {
        // Layout: P(3) UV(2) — normals default to (0,0,1)
        unsigned int numVerts = numOfFloats / 5;
        vtx.reserve(numVerts);
        for (unsigned int i = 0; i < numVerts; ++i) 
        {
            const GLfloat* base = vertices + i * 5;
            VertexPNU v{};
            v.position = { base[0], base[1], base[2] };
            v.normal   = { 0.0f, 0.0f, 1.0f };
            v.uv       = { base[3], base[4] };
            vtx.push_back(v);
        }
    }
    else 
    {
        std::cerr << "[Mesh] Error: unsupported vertex float count " 
                  << numOfFloats << "\\n";
        return;
    }

    std::vector<unsigned int> idx(indices, indices + numOfIndices);
    CreateMesh(vtx, idx);
}

// ----------------------------
// Create from std::vector (preferred)
// ----------------------------
void Mesh::CreateMesh(const std::vector<VertexPNU>& vertices,
                    const std::vector<unsigned int>& indices) 
{
    uploadInterleaved_(vertices, indices);
}

// ----------------------------
// Upload helper (interleaved VBO + EBO)
// ----------------------------
void Mesh::uploadInterleaved_(const std::vector<VertexPNU>& vertices,
                            const std::vector<unsigned int>& indices) 
{
    // Clean previous
    ClearMesh();

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexPNU)),
                vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                indices.data(), GL_STATIC_DRAW);

    const GLsizei stride = static_cast<GLsizei>(sizeof(VertexPNU));

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(VertexPNU,position)); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(VertexPNU,uv));       glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(VertexPNU,normal));   glEnableVertexAttribArray(2);


    indexCount = static_cast<GLsizei>(indices.size());

    // unbind VAO (EBO remains associated with VAO)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ----------------------------
// .OBJ loader (robust tuples + dedup)
// ----------------------------
namespace 
{
    struct Triple 
    { 
        int v, vt, vn;  // already fixed: 0-based or -1
        bool operator==(const Triple& o) const { return v==o.v && vt==o.vt && vn==o.vn; }
    };
    struct TripleHash 
    {
        size_t operator()(const Triple& t) const 
        {
            return (size_t)((t.v+1)*73856093) ^ (size_t)((t.vt+1)*19349663) ^ (size_t)((t.vn+1)*83492791);
        }
    };


    // Convert OBJ 1-based (and negative) index to 0-based
    inline int fixIndex(int idx, int count) 
    {
        return (idx > 0) ? (idx - 1) : (count + idx); // negative index: -1 = last
    }
}

void computeNormals(std::vector<VertexPNU>& vertices, const std::vector<unsigned int>& indices)
{
    // Initialize all normals to zero
    for (auto& v : vertices) {
        v.normal = glm::vec3(0.0f);
    }

    // Accumulate face normals to vertices
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 v0 = vertices[i0].position;
        glm::vec3 v1 = vertices[i1].position;
        glm::vec3 v2 = vertices[i2].position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        vertices[i0].normal += faceNormal;
        vertices[i1].normal += faceNormal;
        vertices[i2].normal += faceNormal;
    }

    // Normalize vertex normals
    for (auto& v : vertices) {
        if (glm::length(v.normal) > 0.0f) {
            v.normal = glm::normalize(v.normal);
        } else {
            v.normal = glm::vec3(0.0f, 0.0f, 1.0f); // fallback
        }
    }
}


bool Mesh::CreateMeshFromOBJ(const std::string& path, bool flipV) 
{
    std::ifstream in(path);
    if (!in) 
    {
        std::cerr << "[Mesh] Failed to open OBJ: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> V; V.reserve(1000);
    std::vector<glm::vec2> VT; VT.reserve(1000);
    std::vector<glm::vec3> VN; VN.reserve(1000);

    std::vector<VertexPNU> interleaved; interleaved.reserve(2000);
    std::vector<unsigned int> indices; indices.reserve(6000);
    std::unordered_map<Triple, unsigned int, TripleHash> dedup;

    std::string line;
    while (std::getline(in, line)) 
    {
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::istringstream ss(line);
        std::string tag; ss >> tag;
        if (tag == "v") 
        {
            glm::vec3 p{}; ss >> p.x >> p.y >> p.z; V.push_back(p);
        } 
        else if (tag == "vt") 
        {
            glm::vec2 t{}; ss >> t.x >> t.y; if (flipV) t.y = 1.0f - t.y; VT.push_back(t);
        } 
        else if (tag == "vn") 
        {
            glm::vec3 n{}; ss >> n.x >> n.y >> n.z; VN.push_back(n);
        } 
        else if (tag == "f") 
        {
            // collect the rest of tokens for this face
            std::vector<std::string> fverts; fverts.reserve(8);
            std::string tok;
            while (ss >> tok) { fverts.push_back(tok); }
            if (fverts.size() < 3) continue; // not a valid face

            auto parseOne = [&](const std::string& s, int& iv, int& ivt, int& ivn) -> bool 
            {
                iv = ivt = ivn = -1;
                int parts[3] = {0,0,0}, p = 0;
                std::string cur;
                for (size_t i=0;i<=s.size();++i) 
                {
                    if (i==s.size() || s[i]=='/') 
                    {
                        if (!cur.empty()) { try { parts[p] = std::stoi(cur); } catch (...) { parts[p] = 0; } }
                        cur.clear(); if (i!=s.size() && p<2) ++p;
                    } 
                    else cur.push_back(s[i]);
                }
                auto fixIndex = [](int idx, int count){ if (idx>0) return idx-1; if (idx<0) return count+idx; return -1; };
                iv  = fixIndex(parts[0], (int)V.size());
                ivt = (p>=1) ? fixIndex(parts[1], (int)VT.size()) : -1;
                ivn = (p>=2) ? fixIndex(parts[2], (int)VN.size()) : -1;
                return (iv >= 0); // v is mandatory
            };

            auto addFaceVertex = [&](int iv, int ivt, int ivn)
            {
                Triple key{iv, ivt, ivn};
                auto it = dedup.find(key);
                if (it != dedup.end()) { indices.push_back(it->second); return; }

                VertexPNU out{};
                out.position = V[iv];
                if (ivt >= 0) out.uv = VT[ivt];
                if (ivn >= 0) out.normal = VN[ivn];

                unsigned int id = (unsigned)interleaved.size();
                interleaved.push_back(out);
                dedup.emplace(key, id);
                indices.push_back(id);
            };

            // Triangulate via fan: (v0, vi, vi+1)
            // First, parse all vertices of this polygon to a temp list of indices
            struct FaceV { int v, vt, vn; };
            std::vector<FaceV> fv; fv.reserve(fverts.size());
            for (const auto& s : fverts) 
            {
                int iv=0, ivt=0, ivn=0;
                parseOne(s, iv, ivt, ivn);
                fv.push_back({iv, ivt, ivn});
            }
            for (size_t i = 1; i + 1 < fv.size(); ++i) 
            {
                // triangle: fv[0], fv[i], fv[i+1]
                addFaceVertex(fv[0].v, fv[0].vt, fv[0].vn);
                addFaceVertex(fv[i].v, fv[i].vt, fv[i].vn);
                addFaceVertex(fv[i+1].v, fv[i+1].vt, fv[i+1].vn);
            }
        }
    }

  if (interleaved.empty() || indices.empty()) 
    {
        std::cerr << "[Mesh] OBJ produced no geometry: " << path << "\n";
        return false;
    }

    // AUTO-GENERATE NORMALS IF MISSING
    bool hasNormals = false;
    for (const auto& v : interleaved) {
        if (glm::length(v.normal) > 0.0001f) {
            hasNormals = true;
            break;
        }
    }
    if (!hasNormals) {
        std::cout << "[Mesh] No normals found in OBJ. Computing them automatically...\n";
        computeNormals(interleaved, indices);
    }

    uploadInterleaved_(interleaved, indices);
    return true;
}

// ----------------------------
// Render
// ----------------------------
void Mesh::RenderMesh() const 
{
    if (VAO == 0 || indexCount == 0) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ----------------------------
// Clear GL resources
// ----------------------------
void Mesh::ClearMesh() 
{
    if (IBO != 0) 
    {
        glDeleteBuffers(1, &IBO);
        IBO = 0;
    }
    if (VBO != 0) 
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (VAO != 0) 
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    indexCount = 0;
}