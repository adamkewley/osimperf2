#include <OpenSim/Analyses/StaticOptimization.h>
#include <OpenSim/OpenSim.h>
#include <OpenSim/Simulation/StatesTrajectory.h>
#include <OpenSim/Tools/CMCTool.h>

#include <cstdio>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <functional>
#include <chrono>
#include <ostream>
#include <sstream>
#include <string>
#include <chrono>
#include <ostream>
#include <string>
#include <cstring>

using namespace OpenSim;
using namespace SimTK;

using PerfClock = std::chrono::steady_clock;

namespace
{
    PerfClock::duration runToolIK(
        const std::string& modelFile,
        const std::string& toolFile,
        bool visualize)
    {
        Model model(modelFile);
        if (visualize) { model.setUseVisualizer(true); }

        OpenSim::InverseKinematicsTool tool(toolFile, false);
        tool.setModel(model);

        const auto begin = PerfClock::now();
        tool.run();
        return PerfClock::now() - begin;
    }

    PerfClock::duration runToolCMC(
        const std::string& modelFile,
        const std::string& toolFile,
        bool visualize)
    {
        Model model(modelFile);
        if (visualize) { model.setUseVisualizer(true); }

        CMCTool tool(toolFile, false);
        tool.setModel(model);
        tool.setFinalTime(0.65);

        const std::string label = model.getName() + "_CMC";

        auto before = PerfClock::now();
        tool.run();
        return PerfClock::now() - before;
    }

    PerfClock::duration runToolFwd(
        const std::string& modelFile,
        const std::string& toolFile,
        bool visualize)
    {
        std::cout << "modelFile = " << modelFile << ", toolFile = " << toolFile << "\n";
        ForwardTool tool(toolFile, true, false);

        Model model(modelFile);
        if (visualize) { model.setUseVisualizer(true); }

        tool.setModel(model);

        const std::string label = model.getName() + "_Fwd";
        const auto before = PerfClock::now();
        tool.run();
        return PerfClock::now() - before;
    }

    PerfClock::duration runAndTime(
            const std::string& modelFile,
            bool visualize,
            SimTK::Real finalTime)
    {
        Model model(modelFile);
        model.setUseVisualizer(visualize);
        SimTK::State& s = model.initSystem();
        s.setTime(0.);
        model.realizeReport(s);

        // The choice of integrator/parameters matches what OpenSim Creator and
        // OpenSim GUI do by default.
        SimTK::RungeKuttaMersonIntegrator integrator(model.getSystem());
        integrator.setInternalStepLimit(20000);
        integrator.setMinimumStepSize(1.0e-8    );
        integrator.setMaximumStepSize(1.0);
        integrator.setAccuracy(1.0e-5);
        integrator.setFinalTime(finalTime);
        // integrator.setReturnEveryInternalStep(true);  // so that cancellations/interrupts work
        integrator.initialize(s);

        SimTK::TimeStepper stepper{model.getSystem(), integrator};
        stepper.initialize(s);
        // stepper.setReportAllSignificantStates(true);  // so that cancellations/interrupts work

        const auto before = PerfClock::now();
        stepper.stepTo(finalTime);
        const auto duration = PerfClock::now() - before;

        if (integrator.getTime() < finalTime) {
            throw std::runtime_error("did not reach final time");
        }

        return duration;
    }
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv + 1, argv + argc);
    // Argument options.
    std::string ik;
    std::string cmc;
    std::string fwd;
    std::string model;
    std::string out;
    bool run = false;
    SimTK::Real simTime = SimTK::NaN;
    bool visualize = false;
    // Parse the arguments.
    for (auto it = args.begin(); it < args.end(); it++) {
        if (*it == "--model") { model = *++it; }
        if (*it == "--out") { out = *++it; }
        if (*it == "--cmc") { cmc = *++it; }
        if (*it == "--fwd") { fwd = *++it; }
        if (*it == "--ik") { ik = *++it; }
        run |= *it == "--run";
        if (*it == "--run") { simTime = std::stod(*++it); }
        visualize |= *it == "--viz";
    }
    // Check parsed arguments.
    SimTK_ASSERT_ALWAYS(!model.empty(), "No model selected");

    std::cout << "model = " << model << "\n";
    std::cout << "cmc   = " << cmc << "\n";
    std::cout << "fwd   = " << fwd << "\n";
    std::cout << "run   = " << run << "\n";
    std::cout << "sim   = " << simTime << "\n";
    std::cout << "viz   = " << visualize << "\n";

    #ifdef OPENSIM_REGISTER_TYPES
        RegisterTypes_osimExampleComponents();
        RegisterTypes_osimSimulation();
    #endif

    PerfClock::duration dt{};

    if (run) {
        dt = runAndTime(model, visualize, simTime);
    }

    if (!cmc.empty()) {
        dt = runToolCMC(model, cmc, visualize);
    }

    if (!fwd.empty()) {
        dt = runToolFwd(model, fwd, visualize);
    }

    if (!ik.empty()) {
        dt = runToolIK(model, ik, visualize);
    }

    const double dseconds = std::chrono::duration_cast<std::chrono::duration<double>>(dt).count();

    std::cout << "RESULTS:\n";
    std::cout << "--> dt = " << dseconds << "\n";

    // TODO: output dseconds to somewhere sane

    return 0;
}
