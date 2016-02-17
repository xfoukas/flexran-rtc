# rt-controller
A real-time controller for SD-RAN

Requirements:
* Cmake >= 2.8
* libprotobuf v2.6.1
* boost >= 1.54
* Compiler with support for C++11

To compile run:

cmake . && make

and then to use run:

./build/rt_controller

To compile with the real-time features enabled (recommended), you need to have a lowlatency linux kernel installed (>= 3.14). To compile you need to set the appropriate flag:

cmake -DLOWLATENCY=ON . && make

and then you need to run as superuser:

sudo ./build/rt_controller

This is required, since changing the scheduling policy from normal to real-time policies requires execution as root.