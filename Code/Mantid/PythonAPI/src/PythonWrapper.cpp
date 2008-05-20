#include <vector>

// Boost Includes ==============================================================
#include "boost/python.hpp"
#include "boost/python/suite/indexing/vector_indexing_suite.hpp"
#include "boost/cstdint.hpp"

// Includes ====================================================================
#include "MantidPythonAPI/PythonInterface.h"
#include "MantidAPI/IAlgorithm.h"
#include "MantidAPI/Workspace.h"

// Using =======================================================================
using namespace boost::python;

// Module ======================================================================

typedef std::vector< std::string > string_vec;
typedef std::vector< double > double_vec;

struct Mantid_API_IAlgorithm_Wrapper: Mantid::API::IAlgorithm
{
    Mantid_API_IAlgorithm_Wrapper(PyObject* py_self_, const Mantid::API::IAlgorithm& p0):
        Mantid::API::IAlgorithm(p0), py_self(py_self_) {}

    Mantid_API_IAlgorithm_Wrapper(PyObject* py_self_):
        Mantid::API::IAlgorithm(), py_self(py_self_) {}

    void initialize() {
        call_method< void >(py_self, "initialize");
    }

    bool execute() {
        return call_method< bool >(py_self, "execute");
    }

    bool isInitialized() const {
        return call_method< bool >(py_self, "isInitialized");
    }

    bool isExecuted() const {
        return call_method< bool >(py_self, "isExecuted");
    }

    void setPropertyValue(int p0, int p1) {
        call_method< void >(py_self, "setPropertyValue", p0, p1);
    }

    bool getPropertyValue(int p0) const {
        return call_method< bool >(py_self, "getPropertyValue", p0);
    }

    PyObject* py_self;
};

struct Mantid_API_Workspace_Wrapper: Mantid::API::Workspace
{
    const std::string id() const {
        return call_method< const std::string >(py_self, "id");
    }

    void init(const int& p0, const int& p1, const int& p2) {
        call_method< void >(py_self, "init", p0, p1, p2);
    }

    long int getMemorySize() const {
        return call_method< long int >(py_self, "getMemorySize");
    }

    long int default_getMemorySize() const {
        return Mantid::API::Workspace::getMemorySize();
    }

    int size() const {
        return call_method< int >(py_self, "size");
    }

    int blocksize() const {
        return call_method< int >(py_self, "blocksize");
    }

    std::vector<double,std::allocator<double> >& dataX(const int p0) {
        return call_method< std::vector<double,std::allocator<double> >& >(py_self, "dataX", p0);
    }

    std::vector<double,std::allocator<double> >& dataY(const int p0) {
        return call_method< std::vector<double,std::allocator<double> >& >(py_self, "dataY", p0);
    }

    std::vector<double,std::allocator<double> >& dataE(const int p0) {
        return call_method< std::vector<double,std::allocator<double> >& >(py_self, "dataE", p0);
    }

    std::vector<double,std::allocator<double> >& dataE2(const int p0) {
        return call_method< std::vector<double,std::allocator<double> >& >(py_self, "dataE2", p0);
    }

    int spectraNo(const int p0) const {
        return call_method< int >(py_self, "spectraNo", p0);
    }

    int& spectraNo(const int p0) {
        return call_method< int& >(py_self, "spectraNo", p0);
    }

    const std::vector<double,std::allocator<double> >& dataX(const int p0) const {
        return call_method< const std::vector<double,std::allocator<double> >& >(py_self, "dataX", p0);
    }

    const std::vector<double,std::allocator<double> >& dataY(const int p0) const {
        return call_method< const std::vector<double,std::allocator<double> >& >(py_self, "dataY", p0);
    }

    const std::vector<double,std::allocator<double> >& dataE(const int p0) const {
        return call_method< const std::vector<double,std::allocator<double> >& >(py_self, "dataE", p0);
    }

    const std::vector<double,std::allocator<double> >& dataE2(const int p0) const {
        return call_method< const std::vector<double,std::allocator<double> >& >(py_self, "dataE2", p0);
    }

    PyObject* py_self;
};


#if _WIN32
BOOST_PYTHON_MODULE(MantidPythonAPI)
#else
BOOST_PYTHON_MODULE(libMantidPythonAPI)
#endif
{	
	//Vectors
	class_< string_vec >( "StringVec" )
	.def( vector_indexing_suite< string_vec >() )
        ;
	
	class_< double_vec >( "DoubleVec" )
	.def( vector_indexing_suite< double_vec >() )
	;
	
	register_ptr_to_python< boost::shared_ptr<Mantid::API::Workspace> >();
	register_ptr_to_python< boost::shared_ptr<Mantid::Kernel::Unit> >();
	
	//Mantid stuff
	class_< Mantid::PythonAPI::PythonInterface >("PythonInterface", init<  >())
        .def(init< const Mantid::PythonAPI::PythonInterface& >())
        .def("InitialiseFrameworkManager", &Mantid::PythonAPI::PythonInterface::InitialiseFrameworkManager)
        .def("CreateAlgorithm", &Mantid::PythonAPI::PythonInterface::CreateAlgorithm, return_value_policy< manage_new_object>())
        .def("LoadIsisRawFile", &Mantid::PythonAPI::PythonInterface::LoadIsisRawFile)
        .def("GetXData", &Mantid::PythonAPI::PythonInterface::GetXData, return_value_policy< manage_new_object >())
        .def("GetYData", &Mantid::PythonAPI::PythonInterface::GetYData, return_value_policy< manage_new_object >())
        .def("GetEData", &Mantid::PythonAPI::PythonInterface::GetEData, return_value_policy< manage_new_object >())
        .def("GetE2Data", &Mantid::PythonAPI::PythonInterface::GetE2Data, return_value_policy< manage_new_object >())
        .def("GetAddressXData", &Mantid::PythonAPI::PythonInterface::GetAddressXData)
        .def("GetAddressYData", &Mantid::PythonAPI::PythonInterface::GetAddressYData)
	;
	
	class_< Mantid::API::IAlgorithm, boost::noncopyable, Mantid_API_IAlgorithm_Wrapper >("IAlgorithm", no_init)
        .def("initialize", pure_virtual(&Mantid::API::IAlgorithm::initialize))
        .def("execute", pure_virtual(&Mantid::API::IAlgorithm::execute))
        .def("isInitialized", pure_virtual(&Mantid::API::IAlgorithm::isInitialized))
        .def("isExecuted", pure_virtual(&Mantid::API::IAlgorithm::isExecuted))
        .def("setPropertyValue", pure_virtual(&Mantid::API::IAlgorithm::setPropertyValue))
        .def("getPropertyValue", pure_virtual(&Mantid::API::IAlgorithm::getPropertyValue))
	;

	class_< Mantid::API::Workspace, boost::noncopyable, Mantid_API_Workspace_Wrapper >("Workspace", no_init)
        .def("id", pure_virtual(&Mantid::API::Workspace::id))
        .def("init", pure_virtual(&Mantid::API::Workspace::init))
        .def("getMemorySize", &Mantid::API::Workspace::getMemorySize, &Mantid_API_Workspace_Wrapper::default_getMemorySize)
        .def("size", pure_virtual(&Mantid::API::Workspace::size))
        .def("blocksize", pure_virtual(&Mantid::API::Workspace::blocksize))
        //.def("dataX", pure_virtual((std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) )&Mantid::API::Workspace::dataX), return_value_policy< manage_new_object >())
        //.def("dataY", pure_virtual((std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) )&Mantid::API::Workspace::dataY), return_value_policy< manage_new_object >())
        //.def("dataE", pure_virtual((std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) )&Mantid::API::Workspace::dataE), return_value_policy< manage_new_object >())
        //.def("dataE2", pure_virtual((std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) )&Mantid::API::Workspace::dataE2), return_value_policy< manage_new_object >())
        .def("spectraNo", pure_virtual((int (Mantid::API::Workspace::*)(const int) const)&Mantid::API::Workspace::spectraNo))
        //.def("spectraNo", pure_virtual((int& (Mantid::API::Workspace::*)(const int) )&Mantid::API::Workspace::spectraNo))
        //.def("dataX", pure_virtual((const std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) const)&Mantid::API::Workspace::dataX), return_value_policy< manage_new_object >())
        //.def("dataY", pure_virtual((const std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) const)&Mantid::API::Workspace::dataY), return_value_policy< manage_new_object >())
        //.def("dataE", pure_virtual((const std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) const)&Mantid::API::Workspace::dataE), return_value_policy< manage_new_object >())
        //.def("dataE2", pure_virtual((const std::vector<double,std::allocator<double> >& (Mantid::API::Workspace::*)(const int) const)&Mantid::API::Workspace::dataE2), return_value_policy< manage_new_object >())
        .def("setTitle", &Mantid::API::Workspace::setTitle)
        .def("setComment", &Mantid::API::Workspace::setComment)
        .def("getComment", &Mantid::API::Workspace::getComment, return_value_policy< copy_const_reference >())
        .def("getTitle", &Mantid::API::Workspace::getTitle, return_value_policy< copy_const_reference >())
        .def("XUnit", (const boost::shared_ptr<Mantid::Kernel::Unit>& (Mantid::API::Workspace::*)() const)&Mantid::API::Workspace::XUnit, return_value_policy< copy_const_reference >())
        //.def("XUnit", (boost::shared_ptr<Mantid::Kernel::Unit>& (Mantid::API::Workspace::*)() )&Mantid::API::Workspace::XUnit, return_value_policy< copy_const_reference >())
        .def("isDistribution", (const bool& (Mantid::API::Workspace::*)() const)&Mantid::API::Workspace::isDistribution, return_value_policy< copy_const_reference >())
        //def("isDistribution", (bool& (Mantid::API::Workspace::*)(bool) )&Mantid::API::Workspace::isDistribution, return_value_policy< copy_const_reference >())
    ;


}

