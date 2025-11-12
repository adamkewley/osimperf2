# osimperf2

> Simplified performance monitoring for opensim-core


# Setup & Install

On Ubuntu 24.04:

```bash
# Install OS dependencies
sudo apt-get install $(cat ubuntu24_apt-dependencies.txt)

# Get opensim-core sources
git clone https://github.com/opensim-org/opensim-core

# Create local virtual environment (IDEs prefer this)
python -m venv .venv/
source ./venv/bin/activate # if you plan on running osimperf2 from the calling terminal
```

# Usage

```bash
osimperf2 build-all # builds + installs historical opensim-core distributions (+runner)
osimperf2 build 009995b446fc50aad6768d88db9c0e6c8217f07e  # build just this commit of opensim-core
```

# C++ Development Environment Setup

- Setup & Install the system (see Setup & Install)
- Build + install at least one opensim-core distribution (see Usage)
- Edit the `CMAKE_PREFIX_PATH`s in `CMakePresets.json` to point towards the opensim-core you
  want to develop the C++ against

# Notes

- Does not include Thoracos/ specialization from https://github.com/ComputationalBiomechanicsLab/OpenSimPerfProjects
