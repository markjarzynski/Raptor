# Raptor Engine

This is my implementation of the Raptor Engine from the book Mastering Graphics Programming with Vulkan.

## Notes on EASTL

Warning, EASTL has cyclical submodule dependencies!!! Do not run git submodule update --recursive. Manually, cd into Common/External/EASTL and do a single git submodule {init, update}.