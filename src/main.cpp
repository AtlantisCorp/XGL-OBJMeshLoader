#include <XGL/XGL.h>

#include <fstream>
#include <sstream>
#include <iostream>

struct OBJMeshLoaderError : public XGL::Error 
{ using XGL::Error::Error; };

struct OBJMeshLoader : public XGL::IMeshLoader, public XGL::Class < OBJMeshLoader >
{
    std::string name() const;

    std::shared_ptr < XGL::Mesh > load(const std::string& name, const std::string& filename, const std::shared_ptr < XGL::IRenderer >& rhs);

    struct OBJVertice 
    {
        float x, y, z;
    };

    struct OBJNormal
    {
        float x, y, z;
    };

    struct OBJTexCoord
    {
        float x, y;
    };

    struct OBJTriangle
    {
        uint32_t v[3];
        uint32_t vt[3];
        uint32_t vn[3];
    };

    struct OBJGroup
    {
        std::vector < OBJTriangle > faces;
    };

    struct OBJMesh 
    {
        std::vector < OBJVertice > vertices;
        std::vector < OBJNormal > normals;
        std::vector < OBJTexCoord > texcoords;
        std::map < std::string, OBJGroup > submeshes;
        std::string name;
    };
};

std::string OBJMeshLoader::name() const 
{
    return "OBJMeshLoader";
}

std::shared_ptr < XGL::Mesh > OBJMeshLoader::load(const std::string& name, const std::string& filename, const std::shared_ptr < XGL::IRenderer >& rhs)
{
    std::ifstream stream(filename);
    XGLThrowIf(!stream, OBJMeshLoaderError("Cannot find file '" + filename + "'."));

    OBJMesh obj_mesh;
    obj_mesh.submeshes.emplace(name, OBJGroup());
    OBJGroup* current_group = &obj_mesh.submeshes.rbegin()->second;

    for (std::string line; std::getline(stream, line);)
    {
        std::cout << "OBJMeshLoader: Processing '" << line << "'" << std::endl;
        std::stringstream linestream(line);
        std::string word; linestream >> word;

        if (word == "o" || word == "g")
        {
            std::string name; linestream >> name;
            obj_mesh.submeshes.emplace(name, OBJGroup());
            current_group = &obj_mesh.submeshes.rbegin()->second;
        }

        else if (word == "v")
        {
            OBJVertice v;
            linestream >> v.x >> v.y >> v.z;
            obj_mesh.vertices.push_back(v);
        }

        else if (word == "vt")
        {
            OBJTexCoord vt;
            linestream >> vt.x >> vt.y;
            obj_mesh.texcoords.push_back(vt);
        }

        else if (word == "vn")
        {
            OBJNormal vn;
            linestream >> vn.x >> vn.y >> vn.z;
            obj_mesh.normals.push_back(vn);
        }

        else if (word == "f")
        {
            OBJTriangle face;
            char* pbuf = &line[0];

            for (int i = 0; i < 3; ++i)
            {
                pbuf = strchr(pbuf, ' ');
                pbuf++;

                face.v[i] = 0;
                face.vt[i] = 0;
                face.vn[i] = 0;

                if (sscanf(pbuf, "%d/%d/%d", &face.v[i], &face.vt[i], &face.vn[i]) != 3)
                {
                    if (sscanf(pbuf, "%d//%d", &face.v[i], &face.vn[i]) != 2)
                    {
                        if (sscanf(pbuf, "%d/%d", &face.v[i], &face.vt[i]) != 2)
                        {
                            sscanf(pbuf, "%d", &face.v[i]);
                        }
                    }
                }

                face.v[i]--;
                if (face.vt[i]) face.vt[i]--;
                if (face.vn[i]) face.vn[i]--;
            }

            current_group->faces.push_back(face);
        }
    }

    // Computes another obj_mesh optimized.

    OBJMesh final_mesh;
    std::vector < std::vector < uint32_t > > final_indices;
    final_indices.reserve(obj_mesh.submeshes.size());

    for (auto& group : obj_mesh.submeshes)
    {
        if (!group.second.faces.size())
            continue;
        
        std::vector < uint32_t > indices;
        indices.reserve(group.second.faces.size() * 3);

        for (auto& face : group.second.faces)
        {
            for (int i = 0; i < 3; ++i)
            {
                final_mesh.vertices.push_back(obj_mesh.vertices[face.v[i]]);
                final_mesh.normals.push_back(obj_mesh.normals[face.vn[i]]);
                final_mesh.texcoords.push_back(obj_mesh.texcoords[face.vt[i]]);
                indices.push_back(final_mesh.vertices.size() - 1);
            }
        }

        final_indices.push_back(indices);
    }

    auto mesh = XGL::Mesh::New(name);
    XGL::Vertex vertex({});

    mesh->setBuffer("position", rhs->createBuffer(XGL::BufferRole::Vertex, final_mesh.vertices));;
    vertex.set(XGL::Attrib("position", XGL::Attrib::Float3, sizeof(float) * 3));

    if (final_mesh.texcoords.size())
    {
        mesh->setBuffer("texcoord", rhs->createBuffer(XGL::BufferRole::Vertex, final_mesh.texcoords));
        vertex.set(XGL::Attrib("texcoord", XGL::Attrib::Float2, sizeof(float) * 2));
    }

    if (final_mesh.normals.size())
    {
        mesh->setBuffer("normal", rhs->createBuffer(XGL::BufferRole::Vertex, final_mesh.normals));
        vertex.set(XGL::Attrib("normal", XGL::Attrib::Float3, sizeof(float) * 3));
    }

    mesh->setVertex(vertex);

    for (auto& indices : final_indices)
    {
        auto ibuffer = rhs->createBuffer(XGL::BufferRole::Index, indices);
        auto subMesh = XGL::SubMesh::New(ibuffer, XGL::Elements::Uint32, indices.size(), 0);
        mesh->addSubMesh(subMesh);
    }

    return mesh;
}

struct OBJMeshLoaderPlugin : public XGL::IPlugin
{
    const char* name() const { 
        return "OBJMeshLoaderPlugin"; 
    }

    void inscribe(XGL::Main& mMain) {
        mMain.meshLoaders().add(OBJMeshLoader::New());
    }

    void unscribe(XGL::Main& mMain) {
        mMain.meshLoaders().remove(mMain.meshLoaders().findByName("OBJMeshLoader"));
    }
};

extern "C" XGL::IPlugin* PluginMain(void) 
{
    static OBJMeshLoaderPlugin plugin;
    return &plugin;
}
