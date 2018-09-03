import WinPyService

def worker():
	x = 2 + 2

if __name__ == '__main__':
	service = WinPyService.WinService ('some_name', enablePauseContinue=True)
	service.set_simple_worker(worker)
	service.run()
