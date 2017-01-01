# Bench
Benchmark facility for C++. Very simple to use, single header only, with sound statistics.

## Design goals
* Very simple to use: A single header-only library, simple API
* Provide sound statistics for accurate reasoning
* C++03 support (so we need a customizable timer)
* Gcc & Visual Studio support
* Usable in any unit testing framework (e.g. boost test, gtest)

Inspired by:
* http://www.bfilipek.com/2016/01/micro-benchmarking-libraries-for-c.html
* https://nonius.io/
* https://github.com/google/benchmark