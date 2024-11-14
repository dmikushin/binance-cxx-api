```
⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⠉⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⠀⣀⠀⠀⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣅⠀⠀⣠⣾⣿⣷⣄⠀⢀⣼⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⠟⠉⠻⢿⣷⣾⡿⠛⠉⠻⣿⣷⣿⡿⠋⠈⠻⣿⣿⣿⣿
⣿⣿⣿⣿⣦⣀⣴⣿⡿⢿⣿⣦⣀⣴⣿⡿⢿⣷⣦⣀⣴⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⡋⠀⠀⠙⢿⣿⡿⠋⠀⠀⢹⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠀⠀⠉⠀⠀⣀⣴⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣀⠀⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
```

## Binance C++ API

Library for accessing Binance Bincoin Exchange using web sockets and JSON.

This version is derived from https://github.com/binance-exchange/binacpp with slight fixes:

 * Corrected endless recursion in `BinaCPP::to_string`
 * Commented out flags no longer supported by modern versions of web sockets
 * CMake build system
 * Refactored entire API
 * Maintaining thread safety
 * Added getLastFundingRate, getServerTime

### Prerequisites

```
sudo apt-get install cmake g++
```

### Building

```
git clone --recurse-submodules https://github.com/dmikushin/binance-cxx-api.git
cd binance-cxx-api
mkdir build
cd build/
cmake ..
make
./example
```

