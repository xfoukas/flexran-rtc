# rt-controller
A real-time controller for SD-RAN

Requirements:
* Cmake >= 2.8
* libprotobuf v2.6.1
* boost >= 1.54
* Compiler with support for C++11
* Optionally: Pistache library for RESTful northbound API support

These dependencies can be automatically installed by running the bash script "install_dependencies" located in the tools directory. The current version of the script only provides support for Ubuntu and Arch Linux. For other Linux distributions, please install these packages manually using your package manager.

To compile run:

cmake . && make

and then to use run:

cd build
sudo ./rt_controller

Superuser permissions are required, since changing the scheduling policy from normal to real-time policies requires execution as root.

The controller is configured by default to provide support for real-time applications as long as a Linux kernel >=3.14 is available. To disable real-time support you must run cmake with the LOWLATENCY flag off (i.e. cmake -DLOWLATENCY=OFF . && make)

The controller provides a REST northbound API for controlling the deployed applications using Pistache. Support for this API is enabled by default. If you want to disable this feature you must run cmake with the REST_NORTHBOUND flag off (i.e. cmake -DREST_NORTHBOUND=OFF . && make)

