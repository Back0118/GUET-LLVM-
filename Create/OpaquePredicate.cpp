#include "llvm/Transforms/passtest/OpaquePredicate.h"



static cl::opt<int>
ObfTimes("oploop",
         cl::desc("Choose how many time the -sub pass loops on a function"),
         cl::value_desc("number of times"), cl::init(0), cl::Optional);

namespace{

    struct OpaquePredicate : public FunctionPass{
        static char ID;
        bool op;
        Instruction* nextI;
        Instruction* currentI;
        std::vector<llvm::Instruction*> originalInstructions;
        std::vector<llvm::Instruction*> opInstructions;
        Instruction* tempTerm;
        OpaquePredicate() : FunctionPass(ID) {}
        OpaquePredicate(bool op) : FunctionPass(ID) {this->op = op; OpaquePredicate();}
        void readfunction(Function &F);
        void takeInput(Instruction& I);
        void movecurrentI(llvm::IRBuilder<> builder);
        void moveInstructions(llvm::IRBuilder<> builder);
        void test(Instruction& I);
        void test1(Instruction& I);
        void test2(Instruction& I);
        void RandMod(Instruction& I);
        void Threexplusone(Instruction& I);
        void Axplusb(Instruction& I);
        void PrimeOr(Instruction& I);
        void timeop(Instruction& I);
        void clockop(Instruction& I);
        bool isPrime(int num);
        bool isArithmeticOperation(const Instruction& I);
        bool runOnFunction(Function &F) override {
            // Function *tmp = &F;
            if(op){
                readfunction(F);
                // printinfo(F);
                return true;
            }
            return false;
        }

    };

}

void OpaquePredicate::readfunction(Function &F){
    //  llvm::errs() << "Running subtest pass on Function:"<<F.getName()<<"\n";
    std::vector<Instruction*> instructions;
    for (auto &BB : F){
        for (auto &I : BB){
            if (isArithmeticOperation(I))
                instructions.push_back(&I);
            // if (isbr(I))
            // {
            //     llvm::errs() << "br: " << I << "\n";
            //     instructions.push_back(&I);
            // }
        }
    }
    int time = ObfTimes;
    while (time--)
    {
        for (auto *I : instructions){
            // test2(*I);
            Threexplusone(*I);
            // Axplusb(*I);
            // RandMod(*I);
            // PrimeOr(*I);
            // timeop(*I);
            // clockop(*I);
        }
        
    }
}

void OpaquePredicate::takeInput(Instruction& I){
    currentI = &I;
    opInstructions.push_back(currentI);
    nextI = I.getNextNode();
    if(llvm::isa<llvm::StoreInst>(nextI)){
        opInstructions.push_back(nextI);
        nextI = nextI->getNextNode();
    }
    while (nextI) {
        originalInstructions.push_back(nextI);
        nextI = nextI->getNextNode();
    }
}

void OpaquePredicate::movecurrentI(llvm::IRBuilder<> builder){
    tempTerm = builder.CreateUnreachable();
    for (llvm::Instruction* inst : opInstructions) {
        inst->moveBefore(tempTerm);
    }
    tempTerm->eraseFromParent();
    opInstructions.clear();
}

void OpaquePredicate::moveInstructions(llvm::IRBuilder<> builder){
    tempTerm = builder.CreateUnreachable();
    for (llvm::Instruction* inst : originalInstructions) {
        inst->moveBefore(tempTerm);
    }
    tempTerm->eraseFromParent();
    originalInstructions.clear();
}

void OpaquePredicate::test(Instruction& I){
    llvm::Instruction* nextI = I.getNextNode();
    if (nextI && nextI->getOpcode() == Instruction::Add){
        llvm::LLVMContext& context = I.getContext();
        llvm::IRBuilder<> builder(context);

        std::vector<llvm::Instruction*> originalInstructions;
        while (nextI) {
            originalInstructions.push_back(nextI);
            nextI = nextI->getNextNode();
        }
        builder.SetInsertPoint(&I);
        llvm::Function* func = I.getFunction();
        llvm::BasicBlock* testBlock = llvm::BasicBlock::Create(context, "testBlock", func);
        builder.CreateBr(testBlock);
        builder.SetInsertPoint(testBlock);
        llvm::Instruction* tempTerm = builder.CreateUnreachable();
        for (llvm::Instruction* inst : originalInstructions) {
            inst->moveBefore(tempTerm);
        }
        tempTerm->eraseFromParent();
    }
}


void OpaquePredicate::test1(Instruction& I){
    llvm::Instruction* nextI = I.getNextNode();  
    if (nextI && nextI->getOpcode() == Instruction::Add){
        llvm::LLVMContext& context = I.getContext();
        llvm::IRBuilder<> builder(context);
        std::vector<llvm::Instruction*> originalInstructions;

        llvm::Instruction* addInstr = nextI;  // 保存add指令
        nextI = nextI->getNextNode();  // 更新nextI为add指令后面的指令

        while (nextI) {
            originalInstructions.push_back(nextI);
            nextI = nextI->getNextNode();
        }

        builder.SetInsertPoint(&I);
        uint64_t randNum = (llvm::cryptoutils->get_uint64_t() % 200);
        ConstantInt *co = (ConstantInt *)ConstantInt::get(llvm::Type::getInt64Ty(I.getContext()), randNum);
        

        llvm::Function* func = I.getFunction();
        llvm::BasicBlock* icmpBlock = llvm::BasicBlock::Create(context, "icmpBlock", func);
        llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);
        llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);
        
        builder.CreateBr(icmpBlock);
        builder.SetInsertPoint(icmpBlock);

        // 在icmpBlock中计算co*(co+1)
        llvm::Value* mul = builder.CreateMul(co, builder.CreateAdd(co, ConstantInt::get(Type::getInt64Ty(context), 1)), "mul");
        llvm::Value* mod = builder.CreateSRem(mul, ConstantInt::get(Type::getInt64Ty(context), 2), "mod");


        // 创建icmp指令
        llvm::Value* cond = builder.CreateICmpEQ(mod, ConstantInt::get(Type::getInt64Ty(context), 0), "cond");

        // 根据cond的值跳转到trueBlock或falseBlock
        builder.CreateCondBr(cond, trueBlock, falseBlock);

        // 在falseBlock中插入I的下一个指令
        builder.SetInsertPoint(falseBlock);
        llvm::Instruction* tempTerm = builder.CreateUnreachable();
        addInstr->moveBefore(tempTerm);  // 移动add指令
        builder.CreateBr(trueBlock);
        tempTerm->eraseFromParent();

        // 在trueBlock中插入其余的指令
        builder.SetInsertPoint(trueBlock);
        llvm::Instruction* tempTerm2 = builder.CreateUnreachable();
        for (llvm::Instruction* inst : originalInstructions) {
            inst->moveBefore(tempTerm2);
        }
        tempTerm2->eraseFromParent();
    }
    
}



//x % z = 1
//y % z = 0
//(x * y) % z = 0
void OpaquePredicate::RandMod(Instruction& I){
    takeInput(I);
    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);

    builder.SetInsertPoint(currentI);
    // llvm::Type* int64Type = llvm::Type::getInt64Ty(I.getContext());  // 获取64位整数类型
    llvm::Type* int32Type = llvm::Type::getInt32Ty(I.getContext());
    // 在栈上为整数分配内存
    llvm::AllocaInst* num1Alloca = builder.CreateAlloca(int32Type);
    llvm::AllocaInst* num2Alloca = builder.CreateAlloca(int32Type);
    llvm::AllocaInst* randNumAlloca = builder.CreateAlloca(int32Type);  // 新增：分配一个新的内存

    // 生成一个1-10的随机数和两个0-300范围内的随机数
    uint32_t randNum = (llvm::cryptoutils->get_uint32_t() % 10) + 1;  // 新增：生成一个1-10的随机数
    uint32_t randNum1;
    uint32_t randNum2;
    do {
        randNum1 = llvm::cryptoutils->get_uint32_t() % 301;  // 生成一个0-300范围内的随机数
    } while (randNum1 % randNum != 1);
    do {
        randNum2 = llvm::cryptoutils->get_uint32_t() % 301;  // 生成一个0-300范围内的随机数
    } while (randNum2 % randNum != 0);

    // 将随机数存储到分配的内存中
    builder.CreateStore(ConstantInt::get(int32Type, randNum1), num1Alloca);
    builder.CreateStore(ConstantInt::get(int32Type, randNum2), num2Alloca);
    builder.CreateStore(ConstantInt::get(int32Type, randNum), randNumAlloca);  // 新增：将随机数存储到分配的内存中

    // 用llvm::Value*加载内存中的数
    llvm::Value* num1Load = builder.CreateLoad(num1Alloca);
    llvm::Value* num2Load = builder.CreateLoad(num2Alloca);
    llvm::Value* randNumLoad = builder.CreateLoad(randNumAlloca);  // 新增：加载内存中的随机数

    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* icmpBlock = llvm::BasicBlock::Create(context, "icmpBlock", func);
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);

    builder.CreateBr(icmpBlock);
    builder.SetInsertPoint(icmpBlock);

    llvm::Value* mul = builder.CreateMul(num1Load, num2Load, "mul");  // 计算num1Load*num2Load
    llvm::Value* mod = builder.CreateSRem(mul, randNumLoad, "mod");  // 修改：计算mul除以新数的余数

    llvm::Value* cond = builder.CreateICmpEQ(mod, ConstantInt::get(int32Type, 1), "cond");  // 检查mod是否等于0
    builder.CreateCondBr(cond, trueBlock, falseBlock);

    builder.SetInsertPoint(falseBlock);
    movecurrentI(builder);
    if (!llvm::isa<llvm::BranchInst>(currentI))
    {
        builder.CreateBr(trueBlock);
    }

    builder.SetInsertPoint(trueBlock);
    moveInstructions(builder);
    
}


void OpaquePredicate::Threexplusone(Instruction& I){
   
    takeInput(I);
    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);
    
    builder.SetInsertPoint(currentI);
    llvm::Type* int32Type = llvm::Type::getInt32Ty(I.getContext());
    llvm::AllocaInst* numAlloca = builder.CreateAlloca(int32Type);
    
    uint32_t randNum = (llvm::cryptoutils->get_uint32_t() % 9000) + 1000;

    builder.CreateStore(ConstantInt::get(int32Type, randNum), numAlloca);
    
    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* loopicmpBlock = llvm::BasicBlock::Create(context, "loopicmpBlock", func);
    llvm::BasicBlock* outloopBlock = llvm::BasicBlock::Create(context, "outloopBlock", func);
    llvm::BasicBlock* mod2icmpBlock = llvm::BasicBlock::Create(context, "mod2icmpBlock", func);
    llvm::BasicBlock* ThreexplusoneBlock = llvm::BasicBlock::Create(context, "ThreexplusoneBlock", func);
    llvm::BasicBlock* xDividedBy2Block = llvm::BasicBlock::Create(context, "xDividedBy2Block", func);
    llvm::BasicBlock* runicmpBlock = llvm::BasicBlock::Create(context, "runicmpBlock", func);
    llvm::BasicBlock* runBlock = llvm::BasicBlock::Create(context, "runBlock", func);


    llvm::Value* three = ConstantInt::get(int32Type, 3);
    llvm::Value* two = ConstantInt::get(int32Type, 2);
    llvm::Value* one = ConstantInt::get(int32Type, 1);
    builder.CreateBr(loopicmpBlock);


    builder.SetInsertPoint(loopicmpBlock);
    llvm::Value* randNumLoad = builder.CreateLoad(numAlloca);
    llvm::Value* cond = builder.CreateICmpSGT(randNumLoad, one);
    builder.CreateCondBr(cond, mod2icmpBlock, outloopBlock);

    builder.SetInsertPoint(mod2icmpBlock);
    llvm::Value* mod = builder.CreateSRem(randNumLoad, two);
    llvm::Value* condMod = builder.CreateICmpEQ(mod, one);
    builder.CreateCondBr(condMod, ThreexplusoneBlock, xDividedBy2Block);

    builder.SetInsertPoint(ThreexplusoneBlock);
    llvm::Value* mul = builder.CreateMul(three, randNumLoad);
    llvm::Value* add = builder.CreateAdd(mul, one);
    builder.CreateStore(add, numAlloca);
    builder.CreateBr(runicmpBlock);

    builder.SetInsertPoint(xDividedBy2Block);
    llvm::Value* div = builder.CreateSDiv(randNumLoad, two);
    builder.CreateStore(div, numAlloca);
    builder.CreateBr(runicmpBlock);

    builder.SetInsertPoint(runicmpBlock);
    uint32_t randNum2 = (llvm::cryptoutils->get_uint32_t() % 999) + 2;
    llvm::Value* randNum2Value = ConstantInt::get(int32Type, randNum2);
    llvm::Value* numAllocaLoad = builder.CreateLoad(numAlloca);
    llvm::Value* cond2 = builder.CreateICmpSLT(numAllocaLoad, randNum2Value);
    builder.CreateCondBr(cond2, runBlock, loopicmpBlock);

    builder.SetInsertPoint(runBlock);
    movecurrentI(builder);
    if (!llvm::isa<llvm::BranchInst>(currentI))
    {
        builder.CreateBr(outloopBlock);
    }

    builder.SetInsertPoint(outloopBlock);
    moveInstructions(builder);
}


void OpaquePredicate::Axplusb(Instruction& I){
    takeInput(I);
    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);

    builder.SetInsertPoint(currentI);
    llvm::Type* int32Type = llvm::Type::getInt32Ty(I.getContext());
    llvm::AllocaInst* numAlloca = builder.CreateAlloca(int32Type);
    uint32_t randNum = (llvm::cryptoutils->get_uint32_t() % 9000) + 1000;
    builder.CreateStore(ConstantInt::get(int32Type, randNum), numAlloca);
    llvm::AllocaInst* n = builder.CreateAlloca(int32Type);
    uint32_t randNum2 = (llvm::cryptoutils->get_uint32_t() % 8) + 2;
    builder.CreateStore(ConstantInt::get(int32Type, randNum2), n);
    llvm::AllocaInst* A = builder.CreateAlloca(int32Type);
    builder.CreateStore(ConstantInt::get(int32Type, randNum2+1), A);
    llvm::Value* Aload = builder.CreateLoad(A);
    // std::vector<llvm::Instruction*> numInstructions;
    // for(uint32_t i = randNum2-1; i > 0; --i){
    //     llvm::Value* num = ConstantInt::get(int32Type, i);
    //     numInstructions.push_back(llvm::dyn_cast<llvm::Instruction>(num));
    // }

    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* loopicmpBlock = llvm::BasicBlock::Create(context, "loopicmpBlock", func);
    llvm::BasicBlock* outloopBlock = llvm::BasicBlock::Create(context, "outloopBlock", func);

    std::vector<llvm::BasicBlock*> blocks;
    for(uint32_t i = 1; i < randNum2; ++i) {
        std::string blockName = "Block" + std::to_string(i);
        llvm::BasicBlock* block = llvm::BasicBlock::Create(context, blockName, func);
        blocks.push_back(block);
    }
    llvm::BasicBlock* modAicmpBlock = llvm::BasicBlock::Create(context, "modAicmpBlock", func);
    llvm::BasicBlock* xDividedByABlock = llvm::BasicBlock::Create(context, "xDividedByABlock", func);
    llvm::BasicBlock* runicmpBlock = llvm::BasicBlock::Create(context, "runicmpBlock", func);
    llvm::BasicBlock* runBlock = llvm::BasicBlock::Create(context, "runBlock", func);

    builder.CreateBr(loopicmpBlock);


    builder.SetInsertPoint(loopicmpBlock);
    llvm::Value* randNumLoad = builder.CreateLoad(numAlloca);
    llvm::Value* cond = builder.CreateICmpSGT(randNumLoad, ConstantInt::get(int32Type, 1));
    builder.CreateCondBr(cond, modAicmpBlock, outloopBlock);

    builder.SetInsertPoint(modAicmpBlock);
    llvm::Value* nLoad = builder.CreateLoad(n);
    llvm::Value* mod = builder.CreateSRem(randNumLoad, nLoad);
    llvm::SwitchInst* switchInst = builder.CreateSwitch(mod, xDividedByABlock, randNum2);
    for (uint32_t i = 0; i < blocks.size(); ++i) {
        switchInst->addCase(builder.getInt32(i+1), blocks[i]);
        builder.SetInsertPoint(blocks[i]);
        llvm::Value* num = ConstantInt::get(int32Type, i+1);
        llvm::Value* B = builder.CreateSub(nLoad,num);
        llvm::Value* mul = builder.CreateMul(Aload, B);
        llvm::Value* add = builder.CreateAdd(mul, randNumLoad);
        builder.CreateStore(add, numAlloca);
        builder.CreateBr(runicmpBlock);
    }

    builder.SetInsertPoint(xDividedByABlock);
    llvm::Value* div = builder.CreateSDiv(randNumLoad, Aload);
    builder.CreateStore(div, numAlloca);
    builder.CreateBr(runicmpBlock);

    builder.SetInsertPoint(runicmpBlock);
    uint32_t randNum3 = (llvm::cryptoutils->get_uint32_t() % 900) + 100;
    llvm::Value* randNum3Value = ConstantInt::get(int32Type, randNum3);
    llvm::Value* numAllocaLoad = builder.CreateLoad(numAlloca);
    llvm::Value* cond2 = builder.CreateICmpSLT(numAllocaLoad, randNum3Value);
    builder.CreateCondBr(cond2, runBlock, loopicmpBlock);

    builder.SetInsertPoint(runBlock);
    movecurrentI(builder);
    if (!llvm::isa<llvm::BranchInst>(currentI))
    {
        builder.CreateBr(outloopBlock);
    }

    builder.SetInsertPoint(outloopBlock);
    moveInstructions(builder);

}
void OpaquePredicate::PrimeOr(Instruction& I){
    takeInput(I);

    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);

    builder.SetInsertPoint(currentI);
    llvm::Type* int64Type = llvm::Type::getInt64Ty(I.getContext());
    llvm::AllocaInst* num1Alloca = builder.CreateAlloca(int64Type);
    llvm::AllocaInst* num2Alloca = builder.CreateAlloca(int64Type);
    uint64_t randNum1;
    uint64_t randNum2;

    do {
        randNum1 = llvm::cryptoutils->get_uint64_t() % 10000;  
    } while (!isPrime(randNum1));
    do {
        randNum2 = llvm::cryptoutils->get_uint64_t() % 10000;  
    } while (!isPrime(randNum2)||randNum2==randNum1);

    builder.CreateStore(ConstantInt::get(int64Type, randNum1), num1Alloca);
    builder.CreateStore(ConstantInt::get(int64Type, randNum2), num2Alloca);

    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* icmpBlock = llvm::BasicBlock::Create(context, "icmpBlock", func);   
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);

    builder.CreateBr(icmpBlock);
    builder.SetInsertPoint(icmpBlock);
    llvm::Instruction* value = currentI;
    do{
        value = value->getPrevNode();
    }while (llvm::isa<llvm::BranchInst>(value)|| llvm::isa<llvm::StoreInst>(value));
    llvm::AllocaInst* numa = builder.CreateAlloca(int64Type);
    llvm::AllocaInst* numb = builder.CreateAlloca(int64Type);
    llvm::Value* a = builder.CreatePtrToInt(value, llvm::Type::getInt64Ty(context));
    builder.CreateStore(a, numa);
    llvm::Value* b = builder.CreateNot(a);
    builder.CreateStore(b, numb);

    llvm::Value* NumA = builder.CreateLoad(numa);
    llvm::Value* NumB = builder.CreateLoad(numb);

    // 加载randNum1和randNum2
    llvm::Value* randNum1Val = builder.CreateLoad(num1Alloca);
    llvm::Value* randNum2Val = builder.CreateLoad(num2Alloca);

    // 计算randNum1*a和randNum2*b
    llvm::Value* mul1 = builder.CreateMul(randNum1Val, NumA);
    llvm::Value* mul2 = builder.CreateMul(randNum2Val, NumB);

    // 比较两个结果是否相等
    llvm::Value* cmp = builder.CreateICmpEQ(mul1, mul2);

    // 根据比较结果跳转
    builder.CreateCondBr(cmp, trueBlock, falseBlock);

    builder.SetInsertPoint(falseBlock);
    movecurrentI(builder);
    if (!llvm::isa<llvm::BranchInst>(currentI))
    {
        builder.CreateBr(trueBlock);
    }

    builder.SetInsertPoint(trueBlock);
    moveInstructions(builder);
}
   
void OpaquePredicate::timeop(Instruction& I){
    Module* M = I.getModule();

    // 定义函数类型：i64 (i64*)
    std::vector<Type*> params;
    params.push_back(Type::getInt64PtrTy(M->getContext()));
    FunctionType* funcType = FunctionType::get(Type::getInt64Ty(M->getContext()), params, false);

    // 在模块中插入或获取函数
    Function* timeFunc = cast<Function>(M->getOrInsertFunction("time", funcType));

    // 确保函数声明没有体
    timeFunc->setDoesNotRecurse();

    // 定义sleep函数类型：i32 (i32)
    std::vector<Type*> sleepParams;
    sleepParams.push_back(Type::getInt32Ty(M->getContext()));
    FunctionType* sleepFuncType = FunctionType::get(Type::getInt32Ty(M->getContext()), sleepParams, false);

    // 在模块中插入或获取sleep函数
    Function* sleepFunc = cast<Function>(M->getOrInsertFunction("sleep", sleepFuncType));

    // 确保函数声明没有体
    sleepFunc->setDoesNotRecurse();

    takeInput(I);
    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);

    builder.SetInsertPoint(currentI);
    llvm::Type* int64Type = llvm::Type::getInt64Ty(I.getContext());
    // 创建一个空的i64指针参数
    llvm::Value* arg = llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(M->getContext()));

    // 创建函数调用
    llvm::CallInst* callInst = builder.CreateCall(timeFunc, arg);

    // 将返回的i64值转换为llvm::Value*
    llvm::Value* timeValue = static_cast<llvm::Value*>(callInst);

    // 创建一个i32类型的1作为参数
    llvm::Value* sleepArg = llvm::ConstantInt::get(llvm::Type::getInt32Ty(M->getContext()), 1);
    // 调用sleep函数
    builder.CreateCall(sleepFunc, sleepArg);


    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* icmpBlock = llvm::BasicBlock::Create(context, "icmpBlock", func);
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);

    builder.CreateBr(icmpBlock);
    builder.SetInsertPoint(icmpBlock);
    // 创建另一个函数调用
    llvm::CallInst* callInst2 = builder.CreateCall(timeFunc, arg);
    // 将返回的i64值转换为另一个llvm::Value*
    llvm::Value* timeValue2 = static_cast<llvm::Value*>(callInst2);
    // 比较两个值是否相等
    llvm::Value* isEqual = builder.CreateICmpEQ(timeValue, timeValue2);

    // 根据比较结果跳转到不同的基本块
    builder.CreateCondBr(isEqual, trueBlock, falseBlock);

    builder.SetInsertPoint(falseBlock);
    movecurrentI(builder);
    if (!llvm::isa<llvm::BranchInst>(currentI))
    {
        builder.CreateBr(trueBlock);
    }

    builder.SetInsertPoint(trueBlock);
    moveInstructions(builder);

}

void OpaquePredicate::clockop(Instruction& I){
    Module* M = I.getModule();

    // 定义函数类型：

    FunctionType* funcType = FunctionType::get(Type::getInt64Ty(M->getContext()), false);

    // 在模块中插入或获取函数
    Function* clockFunc = cast<Function>(M->getOrInsertFunction("clock", funcType));
    // 确保函数声明没有体
    clockFunc->setDoesNotRecurse();

    takeInput(I);
    llvm::LLVMContext& context = I.getContext();
    llvm::IRBuilder<> builder(context);
   
    builder.SetInsertPoint(currentI);
    llvm::Type* int64Type = llvm::Type::getInt64Ty(I.getContext());
    // 创建一个调用clock函数的指令
    llvm::Value* callInst = builder.CreateCall(clockFunc, {});
    // 创建一个新的alloca指令，用于存储clock函数的结果
    llvm::AllocaInst* allocaInst = builder.CreateAlloca(int64Type);
    // 创建一个新的store指令，将clock函数的结果存储在allocaInst指向的内存中
    builder.CreateStore(callInst, allocaInst);
    llvm::Function* func = I.getFunction();
    llvm::BasicBlock* icmpBlock = llvm::BasicBlock::Create(context, "icmpBlock", func);
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(context, "trueBlock", func);
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(context, "falseBlock", func);

    builder.CreateBr(icmpBlock);
    builder.SetInsertPoint(icmpBlock);
    // 创建一个新的load指令，从allocaInst指向的内存中加载值
    llvm::Value* loadInst = builder.CreateLoad(allocaInst);
    // 创建一个新的调用clock函数的指令
    llvm::Value* callInst2 = builder.CreateCall(clockFunc, {});
    // 创建一个新的alloca指令，用于存储clock函数的结果
    llvm::AllocaInst* allocaInst2 = builder.CreateAlloca(int64Type);
    // 创建一个新的store指令，将clock函数的结果存储在allocaInst2指向的内存中
    builder.CreateStore(callInst2, allocaInst2);
    // 创建一个新的load指令，从allocaInst2指向的内存中加载值
    llvm::Value* loadInst2 = builder.CreateLoad(allocaInst2);

    llvm::Value* MulInst = builder.CreateMul(loadInst2, loadInst);

    llvm::Value* icmpInst = builder.CreateICmpEQ(MulInst, ConstantInt::get(int64Type, 0));
    // 根据icmp指令的结果跳转到不同的基本块
    builder.CreateCondBr(icmpInst, trueBlock, falseBlock);

    builder.SetInsertPoint(falseBlock);
    movecurrentI(builder);
    if (!llvm::isa<llvm::BranchInst>(currentI))
    {
        builder.CreateBr(trueBlock);
    }

    builder.SetInsertPoint(trueBlock);
    moveInstructions(builder);

}



bool OpaquePredicate::isArithmeticOperation(const Instruction& I){
    if (I.getOpcode() == Instruction::Add || I.getOpcode() == Instruction::Sub || I.getOpcode() == Instruction::Mul || I.getOpcode() == Instruction::SDiv || I.getOpcode() == Instruction::UDiv || I.getOpcode() == Instruction::SRem || I.getOpcode() == Instruction::URem || I.getOpcode() == Instruction::Shl || I.getOpcode() == Instruction::LShr || I.getOpcode() == Instruction::AShr || I.getOpcode() == Instruction::And || I.getOpcode() == Instruction::Or || I.getOpcode() == Instruction::Xor){
        return true;
    }
    return false;
}

bool OpaquePredicate::isPrime(int num) {
    if (num <= 1) {
        return false;
    }
    for (int i = 2; i <= sqrt(num); i++) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}

Pass *llvm::OP(bool test){
    return new OpaquePredicate(test);
}

char OpaquePredicate::ID = 0;
static RegisterPass<OpaquePredicate> X("OpaquePredicate", "OpaquePredicate  Pass");
