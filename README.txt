PHASE2                                  total   - 16h
======

Setting up Intel OpenCL environment.            - 1h
Examining median filtering sample.              - 1h
Setting up development environment on Linux.    - 1h
Implementing the disparity algorithm.           - 5h
Implementing convenience features.              - 2h
Code refactoring.                               - 2h
Unit testing math implementation.               - 2h
Profiling the application.                      - 2h


PHASE3                                                          total   - 17h
======

Some refactorings related to the Pixels class - lambda enumerations     - 2h
    This leads to more elegant code, but the level of abstraction
    might going to be too high to optimize the code.
    On the other hand this abstraction simplifies the parallelization.
Implementing multithreaded processing using std::threads.               - 4h
    Writing a small test project to test out std::threads functionality
    was required.
Simple command line argument logic using cxxopts                        - 2h
    Currently only the number of threads can be given. Planned support
    for window size.
Building the application on Windows                                     - 3h
    A lot of problems encountered. MinGW does not support
    std::threading. Issues with CMake include paths.
    Eventually compiled the code with MSVC -> weird thing is that the
    resulting optimized binary is 50% slower than the gxx binary.
Testing the application on different machines                           - 1h
    Noticed that using 2 threads instead of 1 almost doubles the
    performance, however using 4 instead of 2 is barely faster.
    Possible reason - notebook i5 has only 2 physical cores.
    Tested on a desktop i5 with 4 physical cores
    -> better performance ratio ~3.6 (4thread / 1thread)
Some performance info has been recorded into the folder 'logs'
Caching the mean matrix -> increase performance by ~10%                 - 3h
    implemented, but not working properly yet
Architecture manual SIMD                                                - 2h
    designed, but not yet implemented

