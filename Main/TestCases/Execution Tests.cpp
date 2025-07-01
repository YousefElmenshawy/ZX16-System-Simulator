#include <gtest/gtest.h>
#include "../ZX16_Simulator.h"
#include <sstream>
#include <string>

// Helper to capture cout
class CoutRedirect {
    std::streambuf* old;
    std::ostringstream ss;
public:
    CoutRedirect() : old(std::cout.rdbuf(ss.rdbuf())) {}
    ~CoutRedirect() { std::cout.rdbuf(old); }
    std::string str() const { return ss.str(); }
};

// Helper to run a program and return register dump
void run_sim_and_capture_exec(const std::string& binfile, std::string& reg_dump) {
    ZX16_Simulator sim;
    sim.loadBinaryFile(binfile);
    sim.run();
    {
        CoutRedirect capture;
        sim.dumpRegisters();
        reg_dump = capture.str();
    }
}

// Execution tests (fill in expected register output as needed)
TEST(ZX16SimulatorExec, TC_ZX16_01_Exec) {
    std::string reg_dump;
    run_sim_and_capture_exec("../TestCases/TC-ZX16-01.bin", reg_dump);
    // Check expected register values
    EXPECT_NE(reg_dump.find("t0 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("ra = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("sp = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("s0 = 8"), std::string::npos);
    EXPECT_NE(reg_dump.find("s1 = 3"), std::string::npos);
    EXPECT_NE(reg_dump.find("t1 = -10"), std::string::npos);
    EXPECT_NE(reg_dump.find("a0 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("a1 = 0"), std::string::npos);
}

TEST(ZX16SimulatorExec, TC_ZX16_02_Exec) {
    std::string reg_dump;
    run_sim_and_capture_exec("../TestCases/TC-ZX16-02.bin", reg_dump);
    // Check expected register values
    EXPECT_NE(reg_dump.find("t0 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("ra = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("sp = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("s0 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("s1 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("t1 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("a0 = 512"), std::string::npos);
    EXPECT_NE(reg_dump.find("a1 = 0"), std::string::npos);
}

TEST(ZX16SimulatorExec, TC_ZX16_03_Exec) {
    std::string reg_dump;
    run_sim_and_capture_exec("../TestCases/TC-ZX16-03.bin", reg_dump);
    // Check expected register values
    EXPECT_NE(reg_dump.find("t0 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("ra = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("sp = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("s0 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("s1 = 15"), std::string::npos);
    EXPECT_NE(reg_dump.find("t1 = 0"), std::string::npos);
    EXPECT_NE(reg_dump.find("a0 = 15"), std::string::npos);
    EXPECT_NE(reg_dump.find("a1 = 0"), std::string::npos);
}

