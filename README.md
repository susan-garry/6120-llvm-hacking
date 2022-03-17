# 6120-llvm-hacking

LLVM projects for CS6120 go here. Currently, const_fold contains a pass that performs local constant propagation and pre-emptively
const-folds `mul` instructions. This currently leaves you with a lot of dead code.

To build the pass, navigate to the `const_fold` directory and run

```
mkdir build
cd build
cmake ..
```
