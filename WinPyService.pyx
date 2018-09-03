# distutils: language = c++
"""
	Copyright (c) 2018 Yaron Vanhulst

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

	See LICENSE.MD for more info.
"""

cdef extern from "Service.hpp":
	void c_init(const char *name, bint )
	bint registerService()
	ctypedef void (*simple_worker_callback)()
	void setSimpleWorker(simple_worker_callback)

##### Exceptions ########################################
class ServiceFailedToRegister(Exception):
	pass

class MissingWorker(Exception):
	pass

##### Classes ###########################################
class WinService:
	def __init__(self, str name, bint enablePauseContinue=False):
		self.worker_is_set = False
		c_init(_str_to_cstr(name), enablePauseContinue)

	def set_simple_worker(self, callback):
		global worker
		worker = callback
		cdef simple_worker_callback c_callback = simple_worker_wrapper
		setSimpleWorker(c_callback)
		self.worker_is_set = True

	def run(self):
		if self.worker_is_set:
			if not registerService(): # Call register, which kickstarts our C service code.
				raise ServiceFailedToRegister

		else:
			raise MissingWorker

##### helpers ###########################################
cdef char* _str_to_cstr(str name):
	cdef bytes py_bytes = name.encode()
	cdef char* c_name = py_bytes
	return c_name

##### callbacks #########################################
# Define as empty function to avoid NoneType Errors
def worker():
	pass

cdef void simple_worker_wrapper():
	worker()
