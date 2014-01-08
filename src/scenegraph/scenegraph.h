#ifndef SCENEGRAPH_H
#define SCENEGRAPH_H

// open space includes
#include "object.h"
#include "scenegraph/scenegraphnode.h"

// std includes
#include <vector>
#include <map>

// ghoul includes
#include "ghoul/opengl/programobject.h"

namespace openspace {

class SceneGraph: public Object {
public:

	// constructors & destructor
	SceneGraph();
	~SceneGraph();

	void init();

	void update();
	void evaluate(Camera *camera);
	void render(Camera *camera);

    void printChildren() const {
        root_->print();
    }

    SceneGraphNode* root() const { return root_; }

private:

	SceneGraphNode *root_;
	std::vector<SceneGraphNode*> nodes_;
	std::map<std::string, ghoul::opengl::ProgramObject*> shaders_;

};

} // namespace openspace

#endif