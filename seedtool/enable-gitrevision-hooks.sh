#!/bin/bash

# Run this command to enable the generation of the gitversion.h file.
# You only need to run it once, but it's safe to run it again anytime.
# See seedtool/.githooks/README.md for more information.

# Add the target specific hooks to the project hooksPath
git config core.hooksPath seedtool/.githooks

# Run the hooks once to update the gitversion.h file.
(cd .. && seedtool/.githooks/gen_gitrevision)
