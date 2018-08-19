# distutils: language = c++

from libcpp.string cimport string

cdef extern from "WinService.hpp":
	cdef cppclass WindowsService:
		WindowsService(string _name, bint _canPauseContinue)

cdef class WinService:
	cdef str name
	cdef WindowsService *_this
	def __cinit__(self,str _name, bint _canPauseContinue):
		self.name = _name
		cdef string cpp_name = _name.encode('UTF-8')
		self._this = new WindowsService(cpp_name, _canPauseContinue)
