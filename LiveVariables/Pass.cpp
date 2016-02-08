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

struct AssignSetsVisitor : public InstVisitor<AssignSetsVisitor> {
  DenseMap<Instruction *, SetVector<Value *>> GenSets;
  DenseMap<Instruction *, SetVector<Value *>> KillSets;

  void addOperandsToGen(Instruction &I) {
    for (const auto &Op : I.operands()) {
      Value *V = Op.get();
      if (isa<Constant>(V)) { continue; }
      GenSets[&I].insert(V);
    }
  }

  void visitAllocaInst(AllocaInst &I) {
    KillSets[&I].insert(&I);
  }
  void visitBinaryOperator(BinaryOperator &I) {
    KillSets[&I].insert(&I);
    addOperandsToGen(I);
  }
  void visitCallInst(CallInst &I) {
    KillSets[&I].insert(&I);
    addOperandsToGen(I);
  }
  void visitGetElementPtrInst(GetElementPtrInst &I) {
    KillSets[&I].insert(&I);
    GenSets[&I].insert(I.getPointerOperand());
  }
  void visitLoadInst(LoadInst &I) {
    KillSets[&I].insert(&I);
    GenSets[&I].insert(I.getPointerOperand());
  }
  void visitReturnInst(ReturnInst &I) {
    addOperandsToGen(I);
  }
  void visitStoreInst(StoreInst &I) {
    GenSets[&I].insert(I.getPointerOperand());
    GenSets[&I].insert(I.getValueOperand());
  }
};

class LiveVariablesAnalysis : public FunctionPass {
  Function *CurrentFunction;
  AssignSetsVisitor Visitor;

public:
  static char ID;
  LiveVariablesAnalysis() : FunctionPass(ID) {}

  DenseMap<BasicBlock *, SetVector<Value *>> InSets;
  DenseMap<Instruction *, SetVector<Value *>> OutSets;

  SetVector<Value *> setUnion(SetVector<Value *> &LHS,
                              SetVector<Value *> &RHS) {
    SetVector<Value *> Result = LHS;
    for (auto V : RHS) {
      Result.insert(V);
    }
    return Result;
  }

  void flow(Instruction *I, SetVector<Value *> &In, SetVector<Value *> &Out) {
    Out.clear();
    Out = setUnion(In, Visitor.GenSets[I]);
    for (auto V : Visitor.KillSets[I]) {
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
