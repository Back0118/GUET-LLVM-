**只说重点，没讲到的函数都不重要**

#### addSquareSub

本质是利用差平方公式：
$$
(a+b)(a-b) = a^2 - b^2
$$
将a+b转化成
$$
(a^2-b^2)/a-b
$$
代码如下：

```c++
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
```

subSquareadd和addSquareSub大致相似，不再赘述



#### addToLogic

本质是使用逻辑运算符表示加法

```python
#原式
a = b+c 

#混淆后 
while c != 0:      
    carry = b & c
    b = b ^ c
    c = carry << 1
a = b
```



**addToLogic的实现需要使用循环，在llvm IR中，实现循环必须使用基本块，所以代码较为复杂**



代码：

```c++
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

```

- originalInstructions用于记录需要替换的指令其后方的指令

- 函数的实际作用为使用三个基本块替换原先的指令，首先提取原指令的两个操作数，进行循环判定要跳转的基本块

- icmpBlock：作用是判断变量是否满足循环条件决定是否跳出循环

- trueBlock：执行循环内逻辑运算操作

- falseBlock：跳出循环，执行后续的操作

  

#### andSubstitution

本质是使用德摩根定律，将AND运算转化为OR运算

即A AND B = NOT ((NOT A) OR (NOT B))

代码：

```c++
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
```

orSubstitution和xorSubstitution都使用了德摩根定律，不再赘述

#### ToRemove

ToRemove的作用是标记原先的指令，放入待删除列表

```c++
void subtest::ToRemove(llvm::Instruction& I){
    if (std::find(toRemove.begin(), toRemove.end(), &I) == toRemove.end()) {
        // llvm::errs() <<"TOremove\n\n"<< I << "\n\n";
        toRemove.push_back(&I);
    }
}
```

#### remove

remove的作用是将待删除列表中的指令删除

```c++
void subtest::remove(){

    for (auto* I : toRemove) {
        llvm::errs() <<"remove\n\n"<< *I << "\n\n";
        I->eraseFromParent();
    }
    toRemove.clear();
}
```



#### 总结

- addSquareSub和subSquareadd利用差平方公式进行混淆，操作较为简单，缺点是多次循环使用后会导致数值溢出
- addToLogic使用逻辑运算符表示加法，操作较为复杂，需要使用基本块进行跳转操作
- andSubstitution使用德摩根定律进行混淆
- 使用replaceAllUsesWith替换指令后，原指令并不会被删除，而是留在原IR中，可以选择将其删除

