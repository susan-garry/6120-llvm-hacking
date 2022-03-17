#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constant.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/Instructions.h"
using namespace llvm;

namespace {
  struct FoldAddPass : public FunctionPass {
    static char ID;
    FoldAddPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
		bool changed = false;
		// errs() << "Original Function: " << F << "\n";
		for (auto& B : F) {
			// Maps a pointer to the last instruction that stored here if a constant was stored
			std::map<Value*, StoreInst*> constants;
			for (auto& I : B) {
				// If this instruction stores a constant ...
				if (auto* instr = dyn_cast<StoreInst>(&I)) {
					if (dyn_cast<Constant>(instr->getValueOperand())) {
						// ... add its pointer to the set of pointers where constants are currently stored
						Value* ptr = instr->getPointerOperand();
						constants[ptr] = instr;
					}
					else {
						constants.erase(instr->getPointerOperand());
					}
				}
				//If this instruction loads from a location where a constant is stored
				if (auto* instr = dyn_cast<LoadInst>(&I)) {
					Value* ptr = instr->getPointerOperand();
					if (constants.find(ptr) != constants.end()) { // The destinantion last stored a constant
						StoreInst* store = constants[ptr];
						Constant* c = cast<Constant>(store->getValueOperand());
						instr->replaceAllUsesWith(c);
						// store->eraseFromParent();
						// instr->eraseFromParent();
					changed = true;
					}
				}
				// Constant folding
				if (auto* instr = dyn_cast<BinaryOperator>(&I)) {
					IRBuilder<> builder(instr);
					// If we're dealing with a mul instruction ...
					if (instr->getOpcode() == Instruction::Mul) {
						// ... and both operands are constants...
						if (auto* lhs = dyn_cast<Constant>(instr->getOperand(0))) {
							if (auto* rhs = dyn_cast<Constant>(instr->getOperand(1))) {
								// ... get their values ...
								const APInt& v1 = lhs->getUniqueInteger();
								const APInt& v2 = rhs->getUniqueInteger();
								// ... calculate the value of the expression ...
								const APInt& v = v1.operator*(v2);
								Constant* c = Constant::getIntegerValue(lhs->getType(), v);

								// ... and replace uses of this instruction with the constant
								instr->replaceAllUsesWith(c);

								// instr->eraseFromParent();
								changed = true;
							}
						}
					}
				}
			}
      	}
		// errs() << "Folded Function: " << F << "\n";
      	return changed;
    }
  };
}

char FoldAddPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerFoldAddPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new FoldAddPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerFoldAddPass);