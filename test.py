from WinPyService import WinService, lightSleep

def worker():
	x = 2 + 2
	lightSleep(10)

if __name__ == '__main__':
	service = WinService ('some_name', enablePauseContinue=True)
	service.set_simple_worker(worker)
	service.run()
