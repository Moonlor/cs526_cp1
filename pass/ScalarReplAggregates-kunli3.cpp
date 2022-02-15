//===- ScalarReplAggregates.cpp - Scalar Replacement of Aggregates --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This transformation implements the well known scalar replacement of
// aggregates transformation.  This xform breaks up alloca instructions of
// structure type into individual alloca instructions for
// each member (if possible).  Then, if possible, it transforms the individual
// alloca instructions into nice clean scalar SSA form.
//
// This combines an SRoA algorithm with Mem2Reg because they
// often interact, especially for C++ programs.  As such, this code
// iterates between SRoA and Mem2Reg until we run out of things to promote.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "scalarrepl"
#include <iostream>
#include "llvm/Transforms/Scalar.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Support/Debug.h"
#include "llvm/ADT/Statistic.h"

using namespace llvm;

STATISTIC(NumReplaced, "Number of aggregate allocas broken up");
STATISTIC(NumPromoted, "Number of scalar allocas promoted to register");

namespace
{
  struct SROA : public FunctionPass
  {
    static char ID; // Pass identification
    SROA() : FunctionPass(ID) {}

    // Entry point for the overall scalar-replacement pass
    bool runOnFunction(Function &F);

    // getAnalysisUsage - List passes required by this pass.  We also know it
    // will not alter the CFG, so say so.
    virtual void getAnalysisUsage(AnalysisUsage &AU) const
    {
      AU.addRequired<DominatorTreeWrapperPass>();
      AU.setPreservesCFG();
    }

  private:
    // Add fields and helper functions for this pass here.

    bool performScalarRepl(Function &F);
    bool performPromotion(Function &F);
    bool isSafeElementUse(Value *Ptr);
    bool isSafeUseOfAllocation(Instruction *User);
    bool isSafeStructAllocaToPromote(AllocaInst *AI);
  };
}

char SROA::ID = 0;
static RegisterPass<SROA> X("scalarrepl-kunli3",
                            "Scalar Replacement of Aggregates (by kunli3)",
                            false /* does not modify the CFG */,
                            false /* transformation, not just analysis */);

// Public interface to create the ScalarReplAggregates pass.
// This function is provided to you.
FunctionPass *createMyScalarReplAggregatesPass() { return new SROA(); }

//===----------------------------------------------------------------------===//
//                      SKELETON FUNCTION TO BE IMPLEMENTED
//===----------------------------------------------------------------------===//
//
// Function runOnFunction:
// Entry point for the overall ScalarReplAggregates function pass.
// This function is provided to you.

namespace isAllocaPromotableImpl
{
  bool isAllocaPromotable(const llvm::AllocaInst *AI)
  {
    // R1: isFPOrFPVectorTy() || isIntOrIntVectorTy() || isPtrOrPtrVectorTy()
    if (!(AI->getAllocatedType()->isFPOrFPVectorTy() ||
          AI->getAllocatedType()->isIntOrIntVectorTy() ||
          AI->getAllocatedType()->isPtrOrPtrVectorTy()))
      return false;

    // R2: The alloca is only used in a load or store instruction and the instruction satisfies !isVolatile()
    for (const auto &&user : AI->users())
    {
      const auto LI = dyn_cast<LoadInst>(user);
      const auto SI = dyn_cast<StoreInst>(user);

      if (!((LI && LI->isVolatile()) || (SI && SI->isVolatile())))
      {
        return false;
      }
    }

    return true;
  }

}

bool SROA::runOnFunction(Function &F)
{

  bool cfg_changed = false;

  const auto scalar_promotion = [&F, this]
  {
    bool cfg_changed = false;
    auto &&bb = F.getEntryBlock();

    while (true)
    {
      std::vector<AllocaInst *> alloca_worklist{};

      for (auto &&inst : bb)
      {
        if (auto alloca_inst = dyn_cast<AllocaInst>(&inst))
        {
          if (isAllocaPromotable(alloca_inst))
          {
            alloca_worklist.push_back(alloca_inst);
          }
        }
      }

      if (alloca_worklist.empty())
        break;

      cfg_changed = true;
      NumPromoted += alloca_worklist.size();

      // allocas, dominator tree, alias set tracker
      PromoteMemToReg(alloca_worklist, getAnalysis<DominatorTreeWrapperPass>().getDomTree());
    }

    return cfg_changed;
  };

  // Step 2: replace aggregate allocas with scalar allocas (sroa)
  cfg_changed = scalar_promotion();

  bool changed = performPromotion(F);
  while (1)
  {
    bool local_change = performScalarRepl(F);
    if (!local_change)
      break; // No need to repromote if no scalarrepl
    changed = true;
    local_change = performPromotion(F);
    if (!local_change)
      break; // No need to re-scalarrepl if no promotion
  }

  return cfg_changed;
}

bool SROA::performPromotion(Function &F)
{
  // const TargetData &TD = getAnalysis<TargetData>();
  // DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  // DominanceFrontier &DF = getAnalysis<DominanceFrontier>();

  // Get the entry node for the function
  BasicBlock &BB = F.getEntryBlock();
  bool changed = false;

  while (1)
  {
    std::vector<AllocaInst *> alloca_insts{};
    // Find allocas that are safe to promote, by looking at all instructions in the entry node
    for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I)
    {
      if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
      {
        if (isAllocaPromotable(AI))
        {
          alloca_insts.push_back(AI);
          NumPromoted++;
        }
      }
    }

    if (alloca_insts.empty())
    {
      break;
    }
    NumPromoted += alloca_insts.size();
    changed = true;

    // PromoteMemToReg(const std::vector<AllocaInst*> &Allocas, DominatorTree &DT, AliasSetTracker *AST = 0)
    PromoteMemToReg(alloca_insts, getAnalysis<DominatorTreeWrapperPass>().getDomTree());
  }
  return changed;
}

// Runs on all of the malloc/alloca instructions in the function,
// removing them if they are only used by getelementptr instructions.
bool SROA::performScalarRepl(Function &F)
{
  std::vector<AllocaInst *> worklist;

  // Scan the entry basic block, adding AllocaInst to the worklist
  BasicBlock &BB = F.getEntryBlock();
  for (BasicBlock::iterator I = BB.begin(), E = BB.end(); I != E; ++I)
  {
    if (AllocaInst *AI = dyn_cast<AllocaInst>(I))
    {
      worklist.push_back(AI);
    }
  }

  // Process the worklist
  bool changed = false;
  while (!worklist.empty())
  {
    AllocaInst *AI = worklist.back();
    worklist.pop_back();

    if (!isa<StructType>(AI->getAllocatedType())) // skip if not struct
      continue;

    // Check that all of the users of the allocation are capable of being transformed.
    // if (!isSafeStructAllocaToPromote(AI))
    //   continue;

    // LLVM_DEBUG(dbgs << "Found inst to xform: " << *AI);
    changed = true;

    // S1: get alloca inst for sub fields;
    std::vector<User *> substitution_cadidates = {};
    substitution_cadidates.reserve(AI->getNumUses());
    for (const auto &user : AI->users())
    {
      substitution_cadidates.push_back(user);
    }

    std::vector<AllocaInst *> sub_alloca_fields{};
    const StructType *ST = dyn_cast<StructType>(AI->getAllocatedType());
    sub_alloca_fields.reserve(ST->getNumContainedTypes());
    if (ST) {
      sub_alloca_fields.reserve(ST->getNumContainedTypes());
      for (int i = 0, e = ST->getNumContainedTypes(); i != e; ++i)
      {
        AllocaInst *new_AI = new AllocaInst(ST->getContainedType(i), 0,
                                            AI->getName() + "." + std::to_string(i), AI);
        // member_alloca_inst.push_back(new_AI);
        // Add to worklist for recursive processing
        // worklist.push_back(new_AI);
        sub_alloca_fields.push_back(new_AI);
      }
    }
    

    // Expand the getelementptr instructions to use the alloca instructions that we want to use.

    // S2: update the users of the struct alloca;
    for (const auto &user : substitution_cadidates)
    {
      if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(user))
      {
        // GEP form: GEP <ptr>, 0, <cst>
        uint64_t Idx = cast<ConstantInt>(GEPI->getOperand(2))->getZExtValue();

        AllocaInst *alloca_to_use = sub_alloca_fields[Idx];
        Value *replace_value;
        if (GEPI->getNumOperands() <= 3)
        {
          // Do not insert a new getelementptr instruction with zero indices,
          // only to have it optimized out later.
          replace_value = alloca_to_use;
        }
        else
        {
          // Expand the sturct once for a layer, adding getelement ptr instruction to finish the indexing.
          // The ptr may be expanded for next round
          std::string old_name = GEPI->getName(); // use the old name in the new inst
          std::vector<Value *> new_args;
          new_args.push_back(ConstantInt::get(Type::getInt32Ty(F.getContext()), 0));
          new_args.insert(new_args.end(), GEPI->op_begin() + 3, GEPI->op_end());
          GEPI->setName("");
          replace_value = GetElementPtrInst::Create(GEPI->getResultElementType(), alloca_to_use, new_args, old_name, GEPI);
        }
        // Move all of the users over to the new GEP.
        GEPI->replaceAllUsesWith(replace_value);
        // Delete the old GEP
        GEPI->getParent()->getInstList().erase(GEPI);
      }
      else
      {
        auto inst = cast<Instruction>(user);
        for (size_t i = 0; i < inst->getNumOperands(); ++i)
        {
          if (inst->getOperand(i) == AI)
          {
            inst->setOperand(i, sub_alloca_fields[0]);
          }
        }
      }
    }
    // Finally, delete the Alloca instruction
    worklist.erase(std::remove(worklist.begin(), worklist.end(), AI), worklist.end());
    AI->getParent()->getInstList().erase(AI);

    NumReplaced++;
  }
  return changed;
}

// Check to see if this user is an allowed use for an aggregate allocation.
// U1.1
bool SROA::isSafeUseOfAllocation(Instruction *user)
{
  if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(user))
  {
    // The GEP is safe to transform if it is of the form GEP <ptr>, 0, <cst>
    if (GEPI->getNumOperands() <= 2 ||
        !cast<ConstantInt>(GEPI->getOperand(1))->isZero())
    {
      return false;
    }
    else
    {
      for (size_t i = 2; i < GEPI->getNumOperands(); ++i)
      {
        if (!isa<ConstantInt>(GEPI->getOperand(i)))
        {
          return false;
        }
      }
    }
  }
  else
  {
    return false;
  }

  return true;
}

// Check to see if this use is an allowed use for a getelementptr instruction
// U1.2
bool SROA::isSafeElementUse(Value *Ptr)
{
  for (Value::user_iterator I = Ptr->user_begin(), E = Ptr->user_end(); I != E; ++I)
  {
    Instruction *user = cast<Instruction>(*I);
    switch (user->getOpcode())
    {
    case Instruction::Load:
      break;
    case Instruction::Store:
      break;
    case Instruction::GetElementPtr:
    {
      GetElementPtrInst *GEP = cast<GetElementPtrInst>(user);
      if (!isSafeElementUse(GEP))
        return false;
      break;
    }
    default:
      LLVM_DEBUG(dbgs() << "  Transformation preventing inst: " << *user);
      return false;
    }
  }
  return true; // All users look ok :)
}

// Check to see if the specified allocation of a structure can be broken down into elements.
bool SROA::isSafeStructAllocaToPromote(AllocaInst *AI)
{
  for (Value::user_iterator I = AI->user_begin(), E = AI->user_end(); I != E; ++I)
  {
    if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(*I))
    {
      // U1.1
      if (!isSafeUseOfAllocation(cast<Instruction>(*I)))
      {
        LLVM_DEBUG(dbgs() << "[U1.1] Cannot transform: " << *AI << "  due to user: " << *I);
        return false;
      }

      // U1.2
      if (GetElementPtrInst *GEPI = dyn_cast<GetElementPtrInst>(*I))
      {
        if (GEPI->getNumOperands() == 3 && !isSafeElementUse(GEPI))
        {
          return false;
        }
      }
    }
    else if (llvm::ICmpInst *ICI = dyn_cast<ICmpInst>(*I))
    {
      // U2
      if (ICI->getPredicate() == ICmpInst::ICMP_EQ || ICI->getPredicate() == ICmpInst::ICMP_NE)
      {
        if (!dyn_cast<ConstantPointerNull>(ICI->getOperand(0)) && !dyn_cast<ConstantPointerNull>(ICI->getOperand(1)))
          LLVM_DEBUG(dbgs() << "[U1.2] Cannot transform: " << *AI << "  due to user: " << *I);
        return false;
      }
    }
  }
  return true;
}
