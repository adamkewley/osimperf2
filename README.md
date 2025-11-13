# osimperf2

> Simplified performance monitoring for opensim-core

`osimperf2` provides scripts and infrastructure for:

- Building and installing historical commits of `opensim-core`, according to a regular sampling grid (i.e. annually,
  quarterly, monthly) into an installation directory (default `opensim-core_installs/`, see `./opynsim build-all`).
- Building a native performance measurement executable (`osimperf2-runner`), so that measurements can
  skip initialization overhead and use specialized native clocks and profiler triggers (see `osimperf2-runner.cpp`)
- Stage and run a single performance benchmark against a single commit with minimal terminal noise
  (see `./osimperf2 measure`).
- Stage and run a full analysis of many benchmarks against many commits, with the ability to
  continuously update the result CSV as new benchmarks/commits come in (`./osimperf2 full-analysis`).

# Setup & Install

On Ubuntu 24.04:

```bash
# Install OS dependencies
sudo apt-get install $(cat ubuntu24_apt-dependencies.txt)

# Get opensim-core sources
git clone https://github.com/opensim-org/opensim-core

# Create local virtual environment (IDEs prefer this)
python3 -m venv .venv/
source ./venv/bin/activate # if you plan on running osimperf2 from the calling terminal
```

# Usage

```bash
# list all commits that the system should/will/has buil{t,d}
./osimperf2 all-commits

# builds + installs historical opensim-core distributions (and osimperf2-runner)
#
# WARNING: this takes a very very long time.
./osimperf2 build-all

# build + installs a particular opensim-core commit (and osimperf2-runner)
./osimperf2 build 009995b446fc50aad6768d88db9c0e6c8217f07e

# measures the performance of one commit + benchmark pair and prints the runtime
./osimperf2 measure 009995b446fc50aad6768d88db9c0e6c8217f07e RajagopalDrop  # 2>/dev/null  # silence log

# performs a "full analysis" (see analysis.toml - this is what generates plot data)
./osimperf2 full-analysis analysis.toml
```

# C++ Development Environment Setup

- Setup & Install the system (see Setup & Install)
- Build + install at least one opensim-core distribution (see Usage)
- Edit the `CMAKE_PREFIX_PATH`s in `CMakePresets.json` to point towards the opensim-core you
  want to develop the C++ against
- Open it in a C++ IDE that supports automatic configuration via `CMakePresets.json` (e.g. CLion,
  Visual Studio).

# Notes

- Does not include Thoracos/ specialization from https://github.com/ComputationalBiomechanicsLab/OpenSimPerfProjects
