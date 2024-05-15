# 如何创建带 -mllvm 选项的 llvm pass

`-mllvm` 是 LLVM 编译器的一个选项，它允许你传递参数给 LLVM 的底层代码生成器。这个选项通常用于调试或者优化 LLVM 的代码生成 。

## 添加头文件

选择在某个地方创建你的头文件，添加必要的c/c++库，并在命名空间中声明 -mllvm  启动时激活的函数

以  obfuscator/include/llvm/Transforms/Obfuscation/Substitution.h  为例 ：

```c++
#ifndef _SUBSTITUTIONS_H_
#define _SUBSTITUTIONS_H_


// LLVM include
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/CryptoUtils.h"

// Namespace
using namespace llvm;
using namespace std;

namespace llvm {
	Pass *createSubstitution ();
	Pass *createSubstitution (bool flag);
}

#endif

```

## 创建 -mllvm 参数

打开  obfuscator/lib/Transforms/IPO/PassManagerBuilder.cpp ，添加你创建的头文件：

```c++
#include "llvm/Transforms/Obfuscation/Substitution.h"
```

定义一个命令行选项

```
static cl::opt<bool> Substitution("sub", cl::init(false),
                                  cl::desc("Enable instruction substitutions"));
```

- `cl::opt<bool>`: 这是 LLVM 命令行库中的一个模板类，用于定义一个命令行选项。这里的 `<bool>` 指定了这个选项的类型是布尔型，可以修改<>中的内容变成其他类型，例如\<std::string>
- `Substitution`: 这是变量的名称，也是命令行选项的名称
- `"sub"`: 这是命令行选项的标志，用户在命令行中使用这个标志来设置这个选项，使用方式为：-mllvm -sub
- `cl::init(false)`: 这是一个初始化器，指定了这个选项的初始值是 `false`
- `cl::desc("Enable instruction substitutions")`: 这是一个描述器，提供了这个选项的描述信息。



在 populateModulePassManager 函数中使用创建的命令行选项调用头文件的函数

```c++
MPM.add(createSubstitution(Substitution));
```



## 创建pass文件夹

在obfuscator/lib/Transforms/ 目录下创建一个文件夹，例如obfuscator/lib/Transforms/Obfuscation ，在目录中创建

Makefile 、LLVMBuild.txt、CMakeLists.txt



创建CMakeLists.txt：

```cmake
add_llvm_library(LLVMObfuscation
	......
  Substitution.cpp
  	......
  )
```

- `add_llvm_library`: 这是 LLVM 构建系统中的一个函数，用于添加一个新的库。这个函数会处理所有必要的构建目标和依赖关系，以确保库可以被正确地构建和链接

- `LLVMObfuscation`: 这是添加的库的名称

-  `Substitution.cpp`: 这是实现pass的源文件

  

创建LLVMBuild.txt：

```
[component_0]
type = Library
name = Obfuscation
parent = Transforms
library_name = Obfuscation
```

- `[component_0]`: 这是一个组件的标识符。在一个 `LLVMBuild.txt` 文件中，可以定义多个组件，每个组件都有一个唯一的标识符
- `type = Library`: 这指定了组件的类型。在这个例子中，组件的类型是 `Library`，表示这是一个库。

- `name = Obfuscation`: 这是组件的名称。在这个例子中，组件的名称是 `Obfuscation`。
- `parent = Transforms`: 这指定了组件的父组件。在这个例子中，父组件是 `Transforms`。这意味着这个库是 `Transforms` 组件的一部分。
- `library_name = Obfuscation`: 这是库的名称。在这个例子中，库的名称也是 `Obfuscation`。



创建Makefile：

```
LEVEL = ../../..
LIBRARYNAME = LLVMObfuscation
#LOADABLE_MODULE = 1
BUILD_ARCHIVE = 1

include $(LEVEL)/Makefile.common
```

- `LEVEL = ../../..`: 用于指定项目的根目录相对于当前目录的位置
- `LIBRARYNAME = LLVMObfuscation`: 用于指定要构建的库的名称
- `BUILD_ARCHIVE = 1`: 设置了一个变量 `BUILD_ARCHIVE`，其值为 `1`。用于指定要构建的是一个静态库。
- `include $(LEVEL)/Makefile.common`: 包含了一个公共的 Makefile 文件，该文件通常包含了一些通用的构建规则和设置。这里，它的路径是相对于 `LEVEL` 变量指定的目录的。



## 创建pass

在文件夹中创建对应的源文件，以Substitution.cpp为例：

```c++
#include "llvm/Transforms/Obfuscation/Substitution.h"
......
 namespace {

    struct Substitution : public FunctionPass {
    	......
    }
    };

} // end of anonymous namespace   
char Substitution::ID = 0;
static RegisterPass<Substitution> X("substitution", "operators substitution");
Pass *llvm::createSubstitution(bool flag) { return new Substitution(flag); }
......
bool Substitution::runOnFunction(Function &F){
    ......
}
......    
```

- `struct Substitution : public FunctionPass`: 这是 `Substitution` 结构体的定义，它继承自 `FunctionPass`，是一个函数级别的 pass。

- `char Substitution::ID = 0;`: 这行代码定义了 `Substitution` 类的一个静态成员 `ID`，并将其初始化为 `0`。在 LLVM 中，每个 pass 类都需要有一个唯一的 `ID` 成员，用于在 PassManager 中识别这个 pass。
- `static RegisterPass<Substitution> X`: 这行代码创建了一个 `RegisterPass<Substitution>` 对象 `X`，并将其注册为一个 pass。`"substitution"` 是这个 pass 的命令行选项名，`"operators substitution"` 是这个 pass 的描述。



## 在对应的LLVMBuild.txt中添加pass

在父目录的LLVMBuild.txt添加pass：

```
[common]
subdirectories = ...... Obfuscation ......
```

在同目录的IPO文件夹的LLVMBuild.txt添加pass：

```
[component_0]
......
required_libraries = ...... Obfuscation ......
```



