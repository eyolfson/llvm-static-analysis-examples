#include "llvm/Pass.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

class GenKillVisitor : public InstVisitor<GenKillVisitor> {
  DenseMap<Instruction *, SetVector<Value *>> GenSets;
  DenseMap<Instruction *, SetVector<Value *>> KillSets;

  void addResultToKill(Instruction &I) {
    KillSets[&I].insert(&I);
  }
  void addOperandsToGen(Instruction &I) {
    for (const auto &Op : I.operands()) {
      Value *V = Op.get();
      if (isa<Constant>(V)) { continue; }
      else if (isa<BasicBlock>(V)) { continue; }
      GenSets[&I].insert(V);
    }
  }

public:
  const SetVector<Value *>& getGenSet(Instruction *I) {
    return GenSets[I];
  }
  const SetVector<Value *>& getKillSet(Instruction *I) {
    return KillSets[I];
  }

  /* Terminator Instructions */
  void visitBranchInst(BranchInst &I) {
    addOperandsToGen(I);
  }
  void visitIndirectBrInst(IndirectBrInst &I) {
    addOperandsToGen(I);
  }
  void visitInvokeInst(InvokeInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitResumeInst(ResumeInst &I) {
    addOperandsToGen(I);
  }
  void visitReturnInst(ReturnInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitSwitchInst(SwitchInst &I) {
    addOperandsToGen(I);
  }
  void visitUnreachableInst(UnreachableInst &I) {
  }

  /* Binary Operations */
  void visitBinaryOperator(BinaryOperator &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }

  /* Vector Operations */
  void visitExtractElementInst(ExtractElementInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitInsertElementInst(InsertElementInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitShuffleVectorInst(ShuffleVectorInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }

  /* Aggregate Operations */
  void visitExtractValueInst(ExtractValueInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitInsertValueInst(InsertValueInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }

  /* Memory Access and Addressing Operations */
  void visitAllocaInst(AllocaInst &I) {
    addResultToKill(I);
  }
  void visitAtomicCmpXchgInst(AtomicCmpXchgInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitAtomicRMWInst(AtomicRMWInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFenceInst(FenceInst &I) {
  }
  void visitGetElementPtrInst(GetElementPtrInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitLoadInst(LoadInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitStoreInst(StoreInst &I) {
    addOperandsToGen(I);
  }

  /* Conversion Operations */
  void visitAddrSpaceCastInst(AddrSpaceCastInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitBitCastInst(BitCastInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFPExtInst(FPExtInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFPToSIInst(FPToSIInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFPToUIInst(FPToUIInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFPTruncInst(FPTruncInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitIntToPtrInst(IntToPtrInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitPtrToIntInst(PtrToIntInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitSExtInst(SExtInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitSIToFPInst(SIToFPInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitTruncInst(TruncInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitUIToFPInst(UIToFPInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitZExtInst(ZExtInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }

  /* Other Operations */
  void visitCallInst(CallInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFCmpInst(FCmpInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitICmpInst(ICmpInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitLandingPadInst(LandingPadInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitPHINode(PHINode &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitSelectInst(SelectInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitVAArgInst(VAArgInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
};

class LiveVariablesAnalysis : public FunctionPass {
  Function *CurrentFunction;
  GenKillVisitor Visitor;
  DenseMap<BasicBlock *, SetVector<Value *>> InSets;
  DenseMap<Instruction *, SetVector<Value *>> OutSets;

public:
  static char ID;
  LiveVariablesAnalysis() : FunctionPass(ID) {}


  SetVector<Value *> setUnion(const SetVector<Value *> &LHS,
                              const SetVector<Value *> &RHS) {
    SetVector<Value *> Result = LHS;
    for (auto V : RHS) {
      Result.insert(V);
    }
    return Result;
  }

  void flow(Instruction *I, SetVector<Value *> &In, SetVector<Value *> &Out) {
    Out.clear();
    Out = setUnion(In, Visitor.getGenSet(I));
    for (auto V : Visitor.getKillSet(I)) {
      Out.remove(V);
    }
  }

  bool computeOutSets(BasicBlock &BB) {
    SetVector<Value *> newIn;
    for (succ_iterator SI = succ_begin(&BB), SIE = succ_end(&BB); SI != SIE; ++SI) {
      BasicBlock *Succ = *SI;
      Instruction *I = &Succ->getInstList().front();
      assert(I && "Basic Block has invalid terminator");
      if (OutSets.count(I) == 0) {
        continue;
      }
      for (auto V : OutSets[I]) {
        newIn.insert(V);
      }
    }

    /* No new to recompute this basic block */
    if (InSets.count(&BB) != 0 && InSets[&BB] == newIn) {
      return false;
    }

    InSets[&BB] = newIn;

    SetVector<Value *> *In = &InSets[&BB];
    for (auto Iter = BB.rbegin(), IterEnd = BB.rend();
         Iter != IterEnd; ++Iter) {
      Instruction *I = &*Iter;
      auto &Out = OutSets[I];
      flow(I, *In, Out);
      In = &Out;
    }

    return true;
  }

  bool runOnFunction(Function &F) override {
    CurrentFunction = &F;

    Visitor.visit(F);

    bool Running = true;
    while (Running) {
      Running = false;
      for (auto &BB : F.getBasicBlockList()) {
        if (computeOutSets(BB)) {
          Running = true;
        }
      }
    }

    return false;
  }

  void printSet(raw_ostream &O, const SetVector<Value *> &S) const {
    O << '{';
    bool First = true;
    for (auto V : S) {
      if (First) {
        First = false;
      }
      else {
        O << ", ";
      }
      V->printAsOperand(O, false);
    }
    O << "}\n";
  }

  void print(raw_ostream &O, const Module *M) const override {
    for (auto &BB : CurrentFunction->getBasicBlockList()) {
      O << "  BasicBlock: ";
      BB.printAsOperand(O, false, M);
      O << '\n';
      for (auto &I : BB.getInstList()) {
        O << "    ";
        printSet(O, OutSets.lookup(&I));
        O << "  ";
        I.dump();
      }
      O << "    ";
      printSet(O, InSets.lookup(&BB));
    }
  }
};

}

char LiveVariablesAnalysis::ID = 0;

static RegisterPass<LiveVariablesAnalysis> X(
  "live-variables-analysis", "Live Variables Analysis Pass", false, true);
