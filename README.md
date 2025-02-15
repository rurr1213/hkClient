# HyperKube Client.

These projects use cmake. This means in general creating a build/ directory below the root CMakeList.txt file and then doing cmake.. And then make to build the source.
```
 mkdir build
 cd build
 cmake ..
 make
```
See setup.sh which does and enables debugging.

### Unit testing

Some unit testing has been implements
It uses the Google Test frame work
```
sudo apt-get install libgtest-dev
```

Each project should have a tests subdirectory with its own CMakeLists.txt
that defines the tests.
The test output should be under the main build/tests/ root directory.
Simply go to the correct path location and run the desired tests.
The tests are not run automatically.

