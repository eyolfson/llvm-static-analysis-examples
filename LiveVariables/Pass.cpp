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
    addOperandsToGen(I);
  }

  /* Binary Operations */
  void visitBinaryOperator(BinaryOperator &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }

  /* Vector Operations */
  void visitExtractElementInst(ExtractElementInst &I) {
  }
  void visitInsertElementInst(InsertElementInst &I) {
  }
  void visitShuffleVectorInst(ShuffleVectorInst &I) {
  }

  /* Aggregate Operations */
  void visitExtractValueInst(ExtractValueInst &I) {
  }
  void visitInsertValueInst(InsertValueInst &I) {
  }

  /* Memory Access and Addressing Operations */
  void visitAllocaInst(AllocaInst &I) {
    addResultToKill(I);
  }
  void visitAtomicCmpXchgInst(AtomicCmpXchgInst &I) {
  }
  void visitAtomicRMWInst(AtomicRMWInst &I) {
  }
  void visitFenceInst(FenceInst &I) {
  }
  void visitGetElementPtrInst(GetElementPtrInst &I) {
    GenSets[&I].insert(I.getPointerOperand());
    addResultToKill(I);
  }
  void visitLoadInst(LoadInst &I) {
    GenSets[&I].insert(I.getPointerOperand());
    addResultToKill(I);
  }
  void visitStoreInst(StoreInst &I) {
    GenSets[&I].insert(I.getPointerOperand());
    GenSets[&I].insert(I.getValueOperand());
  }

  /* Conversion Operations */
  void visitAddrSpaceCastInst(AddrSpaceCastInst &I) {
  }
  void visitBitCastInst(BitCastInst &I) {
  }
  void visitFPExtInst(FPExtInst &I) {
  }
  void visitFPToSIInst(FPToSIInst &I) {
  }
  void visitFPToUIInst(FPToUIInst &I) {
  }
  void visitFPTruncInst(FPTruncInst &I) {
  }
  void visitIntToPtrInst(IntToPtrInst &I) {
  }
  void visitPtrToIntInst(PtrToIntInst &I) {
  }
  void visitSExtInst(SExtInst &I) {
  }
  void visitSIToFPInst(SIToFPInst &I) {
  }
  void visitTruncInst(TruncInst &I) {
  }
  void visitUIToFPInst(UIToFPInst &I) {
  }
  void visitZExtInst(ZExtInst &I) {
  }

  /* Other Operations */
  void visitCallInst(CallInst &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitFCmpInst(FCmpInst &I) {
  }
  void visitICmpInst(ICmpInst &I) {
  }
  void visitLandingPadInst(LandingPadInst &I) {
  }
  void visitPHINode(PHINode &I) {
    addOperandsToGen(I);
    addResultToKill(I);
  }
  void visitSelectInst(SelectInst &I) {
  }
  void visitVAArgInst(VAArgInst &I) {
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
    for (pred_iterator PI = pred_begin(&BB), PIE = pred_end(&BB); PI != PIE; ++PI) {
      BasicBlock *Pred = *PI;
      Instruction *T = Pred->getTerminator();
      assert(T && "Basic Block has invalid terminator");
      if (OutSets.count(T) == 0) {
        continue;
      }
      for (auto V : OutSets[T]) {
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
      O << "BB: ";
      BB.printAsOperand(O, false, M);
      O << '\n';
      for (auto &I : BB.getInstList()) {
        printSet(O, OutSets.lookup(&I));
        I.dump();
      }
      printSet(O, InSets.lookup(&BB));
    }
  }
};

}

char LiveVariablesAnalysis::ID = 0;

static RegisterPass<LiveVariablesAnalysis> X(
  "live-variables-analysis", "Live Variables Analysis Pass", false, true);
