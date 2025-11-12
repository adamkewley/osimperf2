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

class Timer {
	public:

    double measureDuration(
            const std::string& label,
            const std::function<void()>& f);

    void storeResult(
            const std::string& label,
            double dt);

    void print(std::ostream& os) const;

private:
    void tick();
    double tock();

    std::ostringstream m_Results;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Tick;
};


void Timer::tick() { m_Tick = std::chrono::high_resolution_clock::now(); }

double Timer::tock() {
    std::chrono::time_point<std::chrono::high_resolution_clock> tock =
        std::chrono::high_resolution_clock::now();
    const double dt = std::chrono::duration<double>(tock - m_Tick).count();
    m_Tick = std::chrono::time_point<std::chrono::high_resolution_clock>::max();
    return dt;
}

double Timer::measureDuration(
        const std::string& label,
        const std::function<void()>& f) {
    tick();
    f();
    const double dt = tock();
    storeResult(
            label, dt
            );
    return dt;
}

void Timer::storeResult(
    const std::string& label,
    double dt)
{
    m_Results << label << "," << dt << "\n";
}

void Timer::print(std::ostream& os) const {
    os << m_Results.str();
    os.flush();
}

void printHeader(const std::string& label, std::ostream& os) {
    std::cout << "\n";
    std::cout << "==============================================================\n";
    std::cout << "====================== " << label << " ====================\n";
    std::cout << "==============================================================\n";
    std::cout << std::endl;
}

Real runToolIK(
        Timer& timer,
        const std::string& modelFile, const std::string& toolFile,
        bool visualize) {

    printHeader("Inverse Kinematics", std::cout);

    Model model(modelFile);
    if (visualize) { model.setUseVisualizer(true); }

    OpenSim::InverseKinematicsTool tool(toolFile, false);
    tool.setModel(model);

    const std::string label = model.getName() + "_IK";
    const Real dt = timer.measureDuration(label, [&]() { tool.run(); });

    return dt;
}

Real runToolCMC(
        Timer& timer,
        const std::string& modelFile, const std::string& toolFile,
        bool visualize) {

    printHeader("Computed Muscle Control", std::cout);

    Model model(modelFile);
    if (visualize) { model.setUseVisualizer(true); }

    CMCTool tool(toolFile, false);
    tool.setModel(model);
    tool.setFinalTime(0.65);

    const std::string label = model.getName() + "_CMC";
    const Real dt = timer.measureDuration(label, [&]() { tool.run(); });

    return dt;
}

Real runToolFwd(Timer& timer, const std::string& modelFile, const std::string& toolFile,
        bool visualize) {

    printHeader("Forward Dynamics", std::cout);

    std::cout << "modelFile = " << modelFile << ", toolFile = " << toolFile << "\n";
    ForwardTool tool(toolFile, true, false);

    Model model(modelFile);
    if (visualize) { model.setUseVisualizer(true); }

    tool.setModel(model);

    const std::string label = model.getName() + "_Fwd";
    const Real dt = timer.measureDuration(label, [&]() { tool.run(); });

    return dt;
}

SimTK::Real runAndTime(
        Timer& timer,
        const std::string& modelFile, bool visualize, SimTK::Real finalTime) {

    printHeader("Run Integrator", std::cout);

    Model model(modelFile);
    model.setUseVisualizer(visualize);
    SimTK::State& s = model.initSystem();
    s.setTime(0.);
    model.realizeReport(s);

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

    const Real dt = timer.measureDuration(
        model.getName() + "_run",
        [&]() { stepper.stepTo(finalTime); });
    std::flush(std::cout);
    if (not integrator.isSimulationOver()) {
        throw std::runtime_error("did not reach final time");
    }
    return dt;
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

    Timer timer;

    if (run) {
        SimTK::Real dt = runAndTime(timer, model, visualize, simTime);

        std::cout << "RESULTS:\n";
        std::cout << "--> dt = " << dt << "\n";
    }

    if (!cmc.empty()) {
        SimTK::Real dt = runToolCMC(timer, model, cmc, visualize);

        std::cout << "RESULTS:\n";
        std::cout << "--> dt = " << dt << "\n";
    }

    if (!fwd.empty()) {
        SimTK::Real dt = runToolFwd(timer, model, fwd, visualize);

        std::cout << "RESULTS:\n";
        std::cout << "--> dt = " << dt << "\n";
    }

    if (!ik.empty()) {
        SimTK::Real dt = runToolIK(timer, model, ik, visualize);

        std::cout << "RESULTS:\n";
        std::cout << "--> dt = " << dt << "\n";
    }

    std::cout << "Printing results to " << out << std::endl;

    std::ofstream oFile(out, std::fstream::app);
    timer.print(oFile);
    oFile.close();

    return 0;
}

