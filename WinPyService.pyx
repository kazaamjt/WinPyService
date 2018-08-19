# distutils: language = c++

from libcpp.string cimport string

cdef extern from "WinService.hpp":
	cdef cppclass WindowsService:
		WindowsService(string _name, bint _canPauseContinue)
		int run()

cdef class WinService:
	cdef str name
	cdef WindowsService *_this
	def __cinit__(self,str _name, bint _canPauseContinue):
		self.name = _name
		cdef string cpp_name = _name.encode('UTF-8')
		self._this = new WindowsService(cpp_name, _canPauseContinue)

	def run(self):
		cdef int return_val = self._this.run()
		if return_val:
			exit()
