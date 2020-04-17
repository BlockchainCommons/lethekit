#!/bin/bash

# Run this command to disable the generation of the gitversion.h file.
# You only need to run it once, but it's safe to run it again anytime.
# See seedtool/.githooks/README.md for more information.

# Remove the target specific hooks from the project hooksPath
git config --unset core.hooksPath
