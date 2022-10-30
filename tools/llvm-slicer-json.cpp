#include "json/json.h"
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>

#include "dg/tools/llvm-slicer-json.h"

#include <fstream>
#include <iostream>
#include <string>

using namespace dg;

Json::Value SlicedInst::json() {
    Json::Value root;
    root["SourceFile"] = std::string(f->getParent()->getSourceFileName());
    root["FunctionName"] = std::string(f->getName());

    const llvm::DebugLoc &debugInfo = i->getDebugLoc();
    if (&debugInfo == 0)
        return root;
    root["LineNumber"] = debugInfo.getLine();
    root["Column"] = debugInfo.getCol();
    return root;
};

void SlicedInst::print() {
    const llvm::DebugLoc &debugInfo = i->getDebugLoc();

    std::cout << "FuncName: " << std::string(f->getName()) << std::endl;
    if (!debugInfo)
        return;
    std::cout << "LineNumber: " << (int) debugInfo.getLine() << std::endl;
}

bool SlicedJson::build() {
    for (llvm::Module::iterator ModIter = m->begin(); ModIter != m->end();
         ++ModIter) {
        llvm::Function *f = llvm::cast<llvm::Function>(ModIter);

        for (llvm::Function::iterator FuncIter = f->begin();
             FuncIter != f->end(); ++FuncIter) {
            llvm::BasicBlock *b = llvm::cast<llvm::BasicBlock>(FuncIter);
            // BasicBlock::iterator --> Instruction
            for (llvm::BasicBlock::iterator BBIter = b->begin();
                 BBIter != b->end(); ++BBIter) {
                llvm::Instruction *i = llvm::cast<llvm::Instruction>(BBIter);
                const llvm::DebugLoc &debugInfo = i->getDebugLoc();
                if (!debugInfo) {
                    continue;
                }
                SlicedInst si(f, b, i);
                appendInst(si);
            }
        }
    }
}

void SlicedJson::appendInst(SlicedInst si) { is->push_back(si); }

bool SlicedJson::saveOutput(std::string path) {
    Json::Value root;
    for (auto si : *is) {
        root.append(si.json());
    }
    Json::StyledWriter writer;
    std::string jsonResult = writer.write(root);
    std::ofstream out(path);
    out << jsonResult;
}

void SlicedJson::print() {
    for (auto si : *is) {
        si.print();
    }
}