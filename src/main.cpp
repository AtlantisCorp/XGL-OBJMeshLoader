#include <XGL/XGL.h>

struct OBJMeshLoader : public XGL::IMeshLoader, public XGL::Class < OBJMeshLoader >
{
    std::string name() const;

    std::shared_ptr < XGL::Mesh > load(const std::string& name, const std::string& filename, const std::shared_ptr < XGL::IRenderer >& rhs);
};

std::string OBJMeshLoader::name() const 
{
    return "OBJMeshLoader";
}

std::shared_ptr < XGL::Mesh > OBJMeshLoader::load(const std::string& name, const std::string& filename, const std::shared_ptr < XGL::IRenderer >& rhs)
{
    return nullptr;
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