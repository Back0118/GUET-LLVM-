#include "llvm/Transforms/passtest/subtest.h"


static cl::opt<int>
ObfTimes("loop",
         cl::desc("Choose how many time the -sub pass loops on a function"),
         cl::value_desc("number of times"), cl::init(0), cl::Optional);

namespace{
    
    struct subtest : public FunctionPass{
        static char ID;
        bool test;
        std::vector<llvm::Instruction*> toRemove;
        subtest() : FunctionPass(ID) {}
        subtest(bool test) : FunctionPass(ID) {this->test = test; subtest();}
        bool isArithmeticOperation(const llvm::Instruction& I);
        bool isLogicalOperation(const llvm::Instruction& I);
        void addIFInstruction(llvm::Instruction& I);
        void DoaddToLogic(Function &F);
        void addToLogic(llvm::Instruction& I);
        void subTomuladd(llvm::Instruction& I);
        void DoaddSquareSub(Function &F);
        void addSquareSub(BinaryOperator *bo);
        void DosubSquareadd(Function &F);
        void subSquareadd(BinaryOperator *bo);
        void ToRemove(llvm::Instruction& I);
        void remove();
        void addRand(llvm::Instruction& I);
        void DoaddRand(Function &F);
        void addNeg(llvm::Instruction& I);
        void andSubstitution(llvm::Instruction& I);
        void orSubstitution(llvm::Instruction& I);
        void xorSubstitution(llvm::Instruction& I);
        void readfunction(Function &F);
        void printinfo(Function &F);
        bool runOnFunction(Function &F) override {
            // Function *tmp = &F;
            if(test){
                readfunction(F);
                // printinfo(F);
                return true;
            }
            return false;
        }
    };
}

// void subtest::printinfo(Function &F){
//     int count = 0;
//     int count1 = 0;
//     llvm::errs() << "Running subtest pass on Function:"<<F.getName()<<"\n";
//     // for (Function::iterator bb = tmp->begin(); bb != tmp->end(); ++bb)
//     // {
//     //     llvm::errs() << "BasicBlock: " << bb->getName() << "\n";
//     //     count++;
//     // }
//     for (auto &BB : F){
        
//         llvm::errs() << "BasicBlock: " << BB.getName() << "\n";
//         // BB.print(llvm::errs());
//         int count2 = 0;
//             for (auto &I : BB){
//             llvm::errs() << "Instruction: " << I << "\n";
//             std::ostringstream addressStream;
//             addressStream << std::hex << std::showbase << reinterpret_cast<void*>(&I);
//             llvm::errs() << "Instruction address: " << addressStream.str() << "\n\n\n\n\n";
            
//             count2++;
//         }
//         llvm::errs() << "Number of Instructions in the BasicBlock: " << count2 << "\n\n";
//         count1+=count2;
//         count++;
//     }
//     llvm::errs() << "Number of BasicBlocks: " << count << "\n";
//     llvm::errs() << "Number of Instructions in the Function: " << count1 << "\n\n";
// }



void subtest::readfunction(Function &F){
    int time = ObfTimes;
        while(time--){
            DoaddSquareSub(F);
            // DoaddRand(F);
            DosubSquareadd(F);
            // DoaddToLogic(F);
        }
    DoaddToLogic(F); 
    // for (auto &BB : F) {
    //     for (auto &I : BB) {
    //         // andSubstitution(I);
    //         orSubstitution(I);
    //     }
    // }
    
    remove();

}

void subtest::ToRemove(llvm::Instruction& I){
    if (std::find(toRemove.begin(), toRemove.end(), &I) == toRemove.end()) {
        // llvm::errs() <<"TOremove\n\n"<< I << "\n\n";
        toRemove.push_back(&I);
    }
}

void subtest::remove(){

    for (auto* I : toRemove) {
        llvm::errs() <<"remove\n\n"<< *I << "\n\n";
        I->eraseFromParent();
    }
    toRemove.clear();
}



bool subtest::isArithmeticOperation(const llvm::Instruction& I){
    if(I.getOpcode() == Instruction::Add || I.getOpcode() == Instruction::Sub || I.getOpcode() == Instruction::Mul || I.getOpcode() == Instruction::SDiv /*|| I.getOpcode() == Instruction::UDiv || I.getOpcode() == Instruction::SRem || I.getOpcode() == Instruction::URem*/){
        return true;
    }
    return false;
}

bool subtest::isLogicalOperation(const llvm::Instruction& I){
    if(I.getOpcode() == Instruction::And || I.getOpcode() == Instruction::Or || I.getOpcode() == Instruction::Xor){
        return true;
    }
    return false;
}

void subtest::addNeg(llvm::Instruction& I){
    if(I.getOpcode() == Instruction::Add){
        
        BinaryOperator *op = NULL;
        op = BinaryOperator::CreateNeg(I.getOperand(1), "", &I);
        op->print(llvm::errs());
        op = BinaryOperator::Create(Instruction::Sub,I.getOperand(0), op,"", &I);
        op->print(llvm::errs());
        I.replaceAllUsesWith(op);
    }
}

void subtest::addRand(llvm::Instruction& I) {
  BinaryOperator *op = NULL;

  if (I.getOpcode() == Instruction::Add) {
    Type *ty = I.getType();
    uint64_t randNum = (llvm::cryptoutils->get_uint64_t() % 2000)-1000;
    ConstantInt *co = (ConstantInt *)ConstantInt::get(ty, randNum);
    op =BinaryOperator::Create(Instruction::Add, I.getOperand(0), co, "", &I);
    op =BinaryOperator::Create(Instruction::Add, op, I.getOperand(1), "", &I);
    op = BinaryOperator::Create(Instruction::Sub, op, co, "", &I);
    I.replaceAllUsesWith(op);

    ToRemove(I);
  }
 
}

void subtest::DoaddRand(Function &F) {
  for (auto &BB : F) {
    for (auto &I : BB) {
        addRand(I);
    }
  }
}

void subtest::andSubstitution(llvm::Instruction& I) {
  BinaryOperator *op = NULL;

  if (I.getOpcode() == Instruction::And) {
    Value* A = I.getOperand(0);
    Value* B = I.getOperand(1);

    // Create NOT instructions
    Value* notA = BinaryOperator::CreateNot(A, "notA", &I);
    Value* notB = BinaryOperator::CreateNot(B, "notB", &I);

    // Create OR instruction
    Value* notA_or_notB = BinaryOperator::CreateOr(notA, notB, "notA_or_notB", &I);

    // Create NOT instruction
    Value* result = BinaryOperator::CreateNot(notA_or_notB, "result", &I);

    // Replace the original AND instruction with the new NOT instruction
    I.replaceAllUsesWith(result);
    

    ToRemove(I);
  }
}

void subtest::orSubstitution(llvm::Instruction& I) {
  BinaryOperator *op = NULL;

  if (I.getOpcode() == Instruction::Or) {
    Value* A = I.getOperand(0);
    Value* B = I.getOperand(1);

    // Create NOT instructions
    Value* notA = BinaryOperator::CreateNot(A, "notA", &I);
    Value* notB = BinaryOperator::CreateNot(B, "notB", &I);

    // Create AND instructions
    Value* notA_and_notB = BinaryOperator::CreateAnd(notA, notB, "notA_and_notB", &I);

    // Create NOT instruction
    Value* result = BinaryOperator::CreateNot(notA_and_notB, "result", &I);

    // Replace the original OR instruction with the new NOT instruction
    I.replaceAllUsesWith(result);

    ToRemove(I);
  }
}


void subtest::xorSubstitution(llvm::Instruction& I) {
  BinaryOperator *op = NULL;
    if (test)
    {}
  if (I.getOpcode() == Instruction::Xor) {
    Value* A = I.getOperand(0);
    Value* B = I.getOperand(1);

    // Create NOT instructions
    Value* notA = BinaryOperator::CreateNot(A, "notA", &I);
    Value* notB = BinaryOperator::CreateNot(B, "notB", &I);

    // Create AND instructions
    Value* A_and_notB = BinaryOperator::CreateAnd(A, notB, "A_and_notB", &I);
    Value* notA_and_B = BinaryOperator::CreateAnd(notA, B, "notA_and_B", &I);

    // Create OR instruction
    Value* result = BinaryOperator::CreateOr(A_and_notB, notA_and_B, "result", &I);

    // Replace the original XOR instruction with the new OR instruction
    I.replaceAllUsesWith(result);

    ToRemove(I);
  }
}





void subtest::addSquareSub(BinaryOperator *bo) {
  BinaryOperator *op = NULL;

  // Create (b*b - c*c) / (b - c)
  if (bo->getOpcode() == Instruction::Add) {
    if(bo->getOperand(0) != bo->getOperand(1)){
        // Create b*b
    BinaryOperator *bb = BinaryOperator::CreateMul(bo->getOperand(0), bo->getOperand(0), "", bo);
    // Create c*c
    BinaryOperator *cc = BinaryOperator::CreateMul(bo->getOperand(1), bo->getOperand(1), "", bo);
    // Create b*b - c*c
    BinaryOperator *numerator = BinaryOperator::CreateSub(bb, cc, "", bo);
    // Create b - c
    BinaryOperator *denominator = BinaryOperator::CreateSub(bo->getOperand(0), bo->getOperand(1), "", bo);
    // Create (b*b - c*c) / (b - c)
    op = BinaryOperator::CreateSDiv(numerator, denominator, "", bo);

    bo->replaceAllUsesWith(op);
    // bo->eraseFromParent();
    }
    ToRemove(*bo);
  }
}

void subtest::DoaddSquareSub(Function &F){
    for (auto &BB : F){
        for (auto &I : BB){
            if(isArithmeticOperation(I)){
                addSquareSub(dyn_cast<BinaryOperator>(&I));
            }
        } 
    }
}

void subtest::subSquareadd(BinaryOperator *bo){
    BinaryOperator *op = NULL;

    // Create (b*b - c*c) / (b+c)
    if (bo->getOpcode() == Instruction::Sub) {
        if(bo->getOperand(0) != bo->getOperand(1)){
            // Create b*b
        BinaryOperator *bb = BinaryOperator::CreateMul(bo->getOperand(0), bo->getOperand(0), "", bo);
        // Create c*c
        BinaryOperator *cc = BinaryOperator::CreateMul(bo->getOperand(1), bo->getOperand(1), "", bo);
        // Create b*b - c*c
        BinaryOperator *numerator = BinaryOperator::CreateSub(bb, cc, "", bo);
        // Create b + c
        BinaryOperator *denominator = BinaryOperator::CreateAdd(bo->getOperand(0), bo->getOperand(1), "", bo);
        // Create (b*b - c*c) / (b + c)
        op = BinaryOperator::CreateSDiv(numerator, denominator, "", bo);
        bo->replaceAllUsesWith(op);
        // bo->eraseFromParent();
        }
        ToRemove(*bo);
    }
    
}

void subtest::DosubSquareadd(Function &F){
    for (auto &BB : F){
        for (auto &I : BB){
            if(isArithmeticOperation(I)){
                subSquareadd(dyn_cast<BinaryOperator>(&I));
            }
        } 
    }
}

void subtest::addIFInstruction(llvm::Instruction& I){
    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);

    std::vector<llvm::Instruction*> originalInstructions;
    llvm::Instruction* nextI = I.getNextNode();
    while (nextI) {
        originalInstructions.push_back(nextI);
        nextI = nextI->getNextNode();
    }

    // 设置插入点为指令 I 的下一条指令
    builder.SetInsertPoint(I.getNextNode());

    // 创建一个在栈上为一个32位整数分配内存的操作
    llvm::AllocaInst* alloca = builder.CreateAlloca(builder.getInt32Ty(), 0, "alloca");
    alloca->setAlignment(4);

    // 创建一个有符号的比较
    llvm::Value* zero = builder.getInt32(0);
    llvm::Value* icmp = builder.CreateICmpSGT(&I, zero, "icmp");

    // 创建三个基本块作为跳转的目标
    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);
    llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(context, "continueBlock", func);

    // 创建一个条件跳转指令
    builder.CreateCondBr(icmp, trueBlock, falseBlock);

    // 在 trueBlock 中为之前分配内存的整数赋值1
    builder.SetInsertPoint(trueBlock);
    llvm::Value* one = builder.getInt32(1);
    builder.CreateStore(one, alloca);

    // 在 trueBlock 中添加一个跳转至 continueBlock 的无条件跳转指令
    builder.CreateBr(continueBlock);

    // 在 falseBlock 中添加一个跳转至 continueBlock 的无条件跳转指令
    builder.SetInsertPoint(falseBlock);
    builder.CreateBr(continueBlock);

    // 将原先的 I 后方的指令放在 continueBlock 后面
    llvm::Instruction* firstInContinueBlock = &*continueBlock->getFirstInsertionPt();
    for (llvm::Instruction* inst : originalInstructions) {
        inst->moveBefore(firstInContinueBlock);
    }
    
}


void subtest::addToLogic(llvm::Instruction& I){

    if (I.getOpcode() == Instruction::Add)
    {
        llvm::LLVMContext& context = I.getContext();
        llvm::IRBuilder<> builder(context);
        std::vector<llvm::Instruction*> originalInstructions;
        llvm::Instruction* nextI = I.getNextNode();
        while (nextI) {
            originalInstructions.push_back(nextI);
            nextI = nextI->getNextNode();
        }
        // 获取 I 的操作数
        llvm::Value* op1 = I.getOperand(0);
        llvm::Value* op2 = I.getOperand(1);

        // 设置插入点为指令 I           
        builder.SetInsertPoint(&I);

        // 创建两个在栈上为一个32位整数分配内存的操作
        llvm::AllocaInst* alloca1 = builder.CreateAlloca(builder.getInt32Ty(), 0, "alloca1");
        llvm::AllocaInst* alloca2 = builder.CreateAlloca(builder.getInt32Ty(), 0, "alloca2");
        alloca1->setAlignment(4);
        alloca2->setAlignment(4);

        // 将操作数存储到分配的内存中
        builder.CreateStore(op1, alloca1);
        builder.CreateStore(op2, alloca2);


        llvm::Function* func = I.getFunction();
        llvm::BasicBlock* icmpBlock = llvm::BasicBlock::Create(context, "icmpBlock", func);
        llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);
        llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);
        // llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(context, "continueBlock", func);
        
        builder.CreateBr(icmpBlock);

        builder.SetInsertPoint(icmpBlock);
        llvm::Value* load1 = builder.CreateLoad(alloca1, "load1");
        llvm::Value* load2 = builder.CreateLoad(alloca2, "load2");
        llvm::Value* icmp = builder.CreateICmpNE(load2, builder.getInt32(0), "icmp");
        builder.CreateCondBr(icmp, trueBlock, falseBlock);

        builder.SetInsertPoint(trueBlock);
        llvm::Value* andOP = builder.CreateAnd(load1, load2, "andOP");
        llvm::Value* xorOP = builder.CreateXor(load1, load2, "xorOP");
        llvm::Value* shlOP = builder.CreateShl(andOP, 1, "shlOP");
        builder.CreateStore(xorOP, alloca1);
        builder.CreateStore(shlOP, alloca2);
        builder.CreateBr(icmpBlock);


        builder.SetInsertPoint(falseBlock);
        llvm::Value* newload= builder.CreateLoad(alloca1, "newload");
        I.replaceAllUsesWith(newload);
        // builder.CreateBr(continueBlock);

        // 添加一个临时的终止指令
        llvm::Instruction* tempTerm = builder.CreateUnreachable();

        // 将原先 I 后方的所有指令都移动到 falseBlock 的末尾
        for (llvm::Instruction* inst : originalInstructions) {
            inst->moveBefore(tempTerm);
        }

        // 删除临时的终止指令
        tempTerm->eraseFromParent();

        ToRemove(I);

    }

    
}

void subtest::DoaddToLogic(Function &F){
    for (auto &BB : F){
        for (auto &I : BB){
            addToLogic(I);
        } 
    }
}

void subtest::subTomuladd(llvm::Instruction& I){
    if (I.getOpcode() == Instruction::Sub)
    {
        BinaryOperator *op = NULL;
        llvm::Value* one = llvm::ConstantInt::get(I.getOperand(0)->getType(), -1);
        op = BinaryOperator::CreateMul(I.getOperand(1),one, "", &I);
        op = BinaryOperator::Create(Instruction::Add,I.getOperand(0), op,"", &I);
        I.replaceAllUsesWith(op);
        ToRemove(I);
    }
}


char subtest::ID = 0;
static RegisterPass<subtest> X("subtest", "subtest  Pass");

Pass *llvm::subtests(bool test){
    return new subtest(test);
}