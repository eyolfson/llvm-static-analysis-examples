#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class LiveVariablesAnalysis : public FunctionPass {
public:
  static char ID;
  LiveVariablesAnalysis() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
    errs().write_escaped(F.getName()) << '\n';
    return false;
  }

  void print(raw_ostream &O, const Module *M) const override {
    O << "TODO\n";
  }
};

}

char LiveVariablesAnalysis::ID = 0;

static RegisterPass<LiveVariablesAnalysis> X(
  "live-variables-analysis", "Live Variables Analysis Pass", false, true);
