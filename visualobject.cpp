#include "visualobject.h"
#include "shader.h"
#include <fstream>

VisualObject::VisualObject()
{
}

VisualObject::~VisualObject()
{
   glDeleteVertexArrays( 1, &mVAO );
   glDeleteBuffers( 1, &mVBO );
}

void VisualObject::init()
{
}

bool VisualObject::writeFile(std::string fileName)
{
    std::ofstream file{fileName, std::ofstream::out};
    if (file)
    {
        file << mVertices.size() << std::endl;
        for (const auto& v : mVertices)
            file << v;

        return true;
    }
    return false;
}
