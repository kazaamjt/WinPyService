import WinPyService

def worker():
	print("it worked")

if __name__ == '__main__':
	service = WinPyService.WinService ('some_name', False)
	service.set_simple_worker(worker)
	service.run()
