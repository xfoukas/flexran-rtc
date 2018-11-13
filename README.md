# FlexRAN-rtc
A real-time controller for SD-RAN. This is the original version of the code as it appeared in the paper published at ACM CoNEXT 2016:

https://dl.acm.org/citation.cfm?id=2999599

Requirements:
* Cmake >= 2.8
* libprotobuf v2.6.1
* boost >= 1.54
* Compiler with support for C++11
* Optionally: Pistache library for RESTful northbound API support

These dependencies can be automatically installed by running the bash script "install_dependencies" located in the tools directory. The current version of the script only provides support for Ubuntu 16.04 and Arch Linux. For other Linux distributions, please install these packages manually using your package manager.

To compile issue the command:
```
./build_flexran_rtc.sh
```
and then to use:
```
./run_flexran_rtc.sh
```
Superuser permissions are required, since changing the scheduling policy from normal to real-time policies requires execution as root.

The controller is configured by default to provide support for real-time applications as long as a Linux kernel >=3.14 is available. To disable real-time support, build the controller with the -r flag:
```
./build_flexran_rtc.sh -r
```

The controller provides a REST northbound API for controlling the deployed applications using Pistache. Support for this API is enabled by default. If you want to disable this feature you must build FlexRAN with the -n flag:
```
./build_flexran_rtc.sh -n
```

The controller is expecting incoming connections from FlexRAN Agents in port 2210 by default. To change the port run the controller using the command
```
./run_flexran_rtc.sh -p <port_number>
```

The REST northbound API of FlexRAN is expecting requests in port 9999 by default. To change the port run the controller using the command
```
./run_flexran_rtc.sh -n <port_number>
```
