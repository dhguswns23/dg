#ifdef ENABLE_JSON
#ifndef DG_TOOLS_LLVM_SLICER_JSON_H_
#define DG_TOOLS_LLVM_SLICER_JSON_H_

#include <llvm/IR/Module.h>

#include <string>
#include <vector>

#include "json/json.h"

namespace dg {

class SlicedInst {
  public:
    llvm::Function *f;
    llvm::BasicBlock *b;
    llvm::Instruction *i;

    SlicedInst(llvm::Function *_f, llvm::BasicBlock *_b, llvm::Instruction *_i)
            : f(_f), b(_b), i(_i){};

    Json::Value json();

    void print();
};

class SlicedJson {
  public:
    llvm::Module *m;
    std::vector<SlicedInst> *is{new std::vector<SlicedInst>()};

    SlicedJson(llvm::Module *m) : m(m){};

    bool build();
    bool saveOutput(std::string path);
    void appendInst(SlicedInst si);

    void print();
};

} // namespace dg

#endif
#endif