import WinPyService

if __name__ == '__main__':
	SERVICE = WinPyService.WinService('Name', False)
	SERVICE.run()
