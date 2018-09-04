# WinPyService

A simple windows service module.  
Currently only supports a single worker.  
Using this module you can run a simple script as windows service.  

## How to use this module

just copy the WinPyService.cp37-win_amd64.pyd to your project directory  
and use it like any other python module.  
(If you are not using Python 3.7, I recommend building the module yourself.)  

Besides that it's very easy to use:

``` python
import WinPyService

def example_worker():
	x = 2 + 2
	print(4) #Print doesn't do anything when running as a service.
	lightSleep(10)

def init():
	service = WinPyService.WinService ('some_name', enablePauseContinue=True)
	service.set_simple_worker(example_worker) # Pass the example_worker function
	service.run()
```

NOTE: Do not use sleep to wait, instead use the interuptable `lightSleep(int seconds)`.

You can now add the service with powershell or sc.exe:  
(Make sure you are running from an elevated prompt)

``` powershell
New-Service -Name 'test' -BinaryPathName 'C:\Program Files\Python37\python.exe path_to_python_script.py'
```

## Building the module

`python.exe .\setup.py build_ext --inplace`
