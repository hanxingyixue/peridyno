#include "PyRigidBodySystem.h"
#include "RigidBody/RigidBody.h"
#include "RigidBody/RigidBodySystem.h"
#include "RigidBody/RigidBodyShared.h"

using InstanceBase = dyno::InstanceBase;
using namespace dyno;

template <typename TDataType>
void declare_rigid_body_system(py::module& m, std::string typestr) {
	using Class = dyno::RigidBodySystem<TDataType>;
	using Parent = dyno::Node;

	std::string pyclass_name = std::string("RigidBodySystem") + typestr;
	py::class_<Class, Parent, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def("add_box", &Class::addBox)
		.def("add_sphere", &Class::addSphere)
		.def("add_tet", &Class::addTet)
		.def("current_topology", &Class::stateTopology, py::return_value_policy::reference)
		.def("state_collisionMask", &Class::stateCollisionMask, py::return_value_policy::reference);
}



void declare_rigid_body_info(py::module& m, std::string typestr) {
	using Class = dyno::RigidBodyInfo;

	std::string pyclass_name = std::string("RigidBodyInfo") + typestr;
	py::class_<Class, std::shared_ptr<Class>> (m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("linear_velocity", &Class::linearVelocity)
		.def_readwrite("angular_velocity", &Class::angularVelocity)
		.def_readwrite("position", &Class::position)
		.def_readwrite("inertia", &Class::inertia)
		.def_readwrite("mass", &Class::mass)
		.def_readwrite("friction", &Class::friction)
		.def_readwrite("restitution", &Class::restitution)
		.def_readwrite("motionType", &Class::motionType)
		.def_readwrite("shapeType", &Class::shapeType)
		.def_readwrite("collisionMask", &Class::collisionMask);
}


void declare_box_info(py::module& m, std::string typestr) {
	using Class = dyno::BoxInfo;

	std::string pyclass_name = std::string("BoxInfo") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("center", &Class::center)
		.def_readwrite("halfLength", &Class::halfLength);

}


void declare_sphere_info(py::module& m, std::string typestr) {
	using Class = dyno::SphereInfo;

	std::string pyclass_name = std::string("SphereInfo") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def_readwrite("center", &Class::center)
		.def_readwrite("radius", &Class::radius);

}



// class: TetInfo      - For Examples_1: QT_Bricks
void declare_tet_info(py::module& m, std::string typestr) {
	using Class = dyno::TetInfo;

	std::string pyclass_name = std::string("TetInfo") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
//		.def_readwrite("v", &Class::v); // "def_readwrite" is not applicable to fixed arrays,so replace it with the get-set method.
		.def_property("v", &get_v, &set_v);
}




// class: TContactPair      - For Examples_1: QT_Bricks
template<typename Real>
void declare_collisionData_TContactPair(py::module& m, std::string typestr) {
	using Class = dyno::TContactPair<Real>;

	std::string pyclass_name = std::string("TContactPair") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def(py::init<int, int, ContactType, Vector<Real, 3>, Vector<Real, 3>, Vector<Real, 3>, Vector<Real, 3>>())
		.def_readwrite("bodyId1", &Class::bodyId1)
		.def_readwrite("bodyId2", &Class::bodyId2)
		.def_readwrite("interpenetration", &Class::interpenetration)
		.def_readwrite("pos1", &Class::pos1)
		.def_readwrite("pos2", &Class::pos2)
		.def_readwrite("normal1", &Class::normal1)
		.def_readwrite("normal2", &Class::normal2)
		.def_readwrite("contactType", &Class::contactType);
}


// class: NeighborElementQuery      - For Examples_1: QT_Bricks  
#include "Collision/NeighborElementQuery.h"
template<typename TDataType>
void declare_neighbor_element_query(py::module& m, std::string typestr) {
	using Class = dyno::NeighborElementQuery<TDataType>;
	std::string pyclass_name = std::string("NeighborElementQuery") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def("in_discreteElements", &Class::inDiscreteElements,py::return_value_policy::reference)
		.def("in_collisionMask", &Class::inCollisionMask, py::return_value_policy::reference)
		.def("out_contacts", &Class::outContacts , py::return_value_policy::reference);
}



// class: ContactsToEdgeSet      - For Examples_1: QT_Bricks
#include "Mapping/ContactsToEdgeSet.h"
template<typename TDataType>
void declare_contacts_to_edge_set(py::module& m, std::string typestr) {
	using Class = dyno::ContactsToEdgeSet<TDataType>;
	using Parent = dyno::TopologyMapping;

	declare_collisionData_TContactPair<Real>(m, ""); //For inContacts:TContactPair<Real>

	std::string pyclass_name = std::string("ContactsToEdgeSet") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def("in_contacts", &Class::inContacts, py::return_value_policy::reference)
		.def("var_scale", &Class::varScale, py::return_value_policy::reference)
		.def("out_edgeSet", &Class::outEdgeSet, py::return_value_policy::reference);
}



// class: ContactsToPointSet3f      - For Examples_1: QT_Bricks
#include "Mapping/ContactsToPointSet.h"
template<typename TDataType>
void declare_contacts_to_point_set(py::module& m, std::string typestr) {
	using Class = dyno::ContactsToPointSet<TDataType>;
	using Parent = dyno::TopologyMapping;
	std::string pyclass_name = std::string("ContactsToPointSet") + typestr;
	py::class_<Class, std::shared_ptr<Class>>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
		.def(py::init<>())
		.def("in_contacts", &Class::inContacts, py::return_value_policy::reference)
		.def("out_pointSet", &Class::outPointSet, py::return_value_policy::reference);
}




void pybind_rigid_body_system(py::module& m) {
	declare_rigid_body_system<dyno::DataType3f>(m, "3f");

	declare_rigid_body_info(m, "");
	declare_box_info(m,"");
	
	declare_sphere_info(m, "");
	declare_tet_info(m, "");

	declare_neighbor_element_query<dyno::DataType3f>(m, "3f");
	declare_contacts_to_edge_set<dyno::DataType3f>(m, "3f");
	declare_contacts_to_point_set<dyno::DataType3f>(m, "3f");

}
