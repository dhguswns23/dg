#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "dg/tools/llvm-slicer-opts.h"
#include "dg/tools/llvm-slicer-preprocess.h"
#include "dg/tools/llvm-slicer-utils.h"
#include "dg/tools/llvm-slicer.h"
#include "dg/llvm/InstructionInfoTable.h"
#include "git-version.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/raw_ostream.h>

#ifdef ENABLE_JSON
#include "dg/tools/llvm-slicer-json.h"
#include <json/json.h>
#endif

#include "dg/ADT/Queue.h"
#include "dg/util/debug.h"

using namespace dg;

using dg::LLVMDataDependenceAnalysisOptions;
using dg::LLVMPointerAnalysisOptions;
using llvm::errs;

using AnnotationOptsT =
        dg::debug::LLVMDGAssemblyAnnotationWriter::AnnotationOptsT;

llvm::cl::opt<bool> enable_debug(
        "dbg", llvm::cl::desc("Enable debugging messages (default=false)."),
        llvm::cl::init(false), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<bool> should_verify_module(
        "dont-verify", llvm::cl::desc("Verify sliced module (default=true)."),
        llvm::cl::init(true), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<bool> remove_unused_only(
        "remove-unused-only",
        llvm::cl::desc("Only remove unused parts of module (default=false)."),
        llvm::cl::init(false), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<bool> statistics(
        "statistics",
        llvm::cl::desc("Print statistics about slicing (default=false)."),
        llvm::cl::init(false), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<bool>
        dump_dg("dump-dg",
                llvm::cl::desc("Dump dependence graph to dot (default=false)."),
                llvm::cl::init(false), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<bool> dump_dg_only(
        "dump-dg-only",
        llvm::cl::desc("Only dump dependence graph to dot,"
                       " do not slice the module (default=false)."),
        llvm::cl::init(false), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<bool> dump_bb_only(
        "dump-bb-only",
        llvm::cl::desc("Only dump basic blocks of dependence graph to dot"
                       " (default=false)."),
        llvm::cl::init(false), llvm::cl::cat(SlicingOpts));

llvm::cl::opt<std::string> annotationOpts(
        "annotate",
        llvm::cl::desc(
                "Save annotated version of module as a text (.ll).\n"
                "Options:\n"
                "  dd: data dependencies,\n"
                "  cd:control dependencies,\n"
                "  pta: points-to information,\n"
                "  memacc: memory accesses of instructions,\n"
                "  slice: comment out what is going to be sliced away).\n"
                "for more options, use comma separated list"),
        llvm::cl::value_desc("val1,val2,..."), llvm::cl::init(""),
        llvm::cl::cat(SlicingOpts));

static void maybe_print_statistics(llvm::Module *M,
                                   const char *prefix = nullptr) {
    if (!statistics)
        return;

    using namespace llvm;
    uint64_t inum, bnum, fnum, gnum;
    inum = bnum = fnum = gnum = 0;

    for (const auto &F : *M) {
        // don't count in declarations
        if (F.empty())
            continue;

        ++fnum;

        for (const BasicBlock &B : F) {
            ++bnum;
            inum += B.size();
        }
    }

    for (auto I = M->global_begin(), E = M->global_end(); I != E; ++I)
        ++gnum;

    if (prefix)
        errs() << prefix;

    errs() << "Globals/Functions/Blocks/Instr.: " << gnum << " " << fnum << " "
           << bnum << " " << inum << "\n";
}


static AnnotationOptsT parseAnnotationOptions(const std::string &annot) {
    if (annot.empty())
        return {};

    AnnotationOptsT opts{};
    std::vector<std::string> lst = splitList(annot);
    for (const std::string &opt : lst) {
        if (opt == "dd")
            opts |= AnnotationOptsT::ANNOTATE_DD;
        else if (opt == "cd" || opt == "cda")
            opts |= AnnotationOptsT::ANNOTATE_CD;
        else if (opt == "dda" || opt == "du")
            opts |= AnnotationOptsT::ANNOTATE_DEF;
        else if (opt == "pta")
            opts |= AnnotationOptsT::ANNOTATE_PTR;
        else if (opt == "memacc")
            opts |= AnnotationOptsT::ANNOTATE_MEMORYACC;
        else if (opt == "slice" || opt == "sl" || opt == "slicer")
            opts |= AnnotationOptsT::ANNOTATE_SLICE;
    }

    return opts;
}

int main(int argc, char *argv[]) {
    setupStackTraceOnError(argc, argv);
    
#if ((LLVM_VERSION_MAJOR >= 6))
    llvm::cl::SetVersionPrinter([](llvm::raw_ostream & /*unused*/) {
        printf("%s\n", GIT_VERSION);
    });
#else
    llvm::cl::SetVersionPrinter([]() { printf("%s\n", GIT_VERSION); });
#endif

    SlicerOptions options = parseSlicerOptions(argc, argv,
                                               /* requireCrit = */ true);

    if (enable_debug) {
        DBG_ENABLE();
    }

    // dump_dg_only implies dumg_dg
    if (dump_dg_only) {
        dump_dg = true;
    }

    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> M =
            parseModule("llvm-slicer", context, options);
    if (!M)
        return 1;

    if (!M->getFunction(options.dgOptions.entryFunction)) {
        llvm::errs() << "The entry function not found: "
                     << options.dgOptions.entryFunction << "\n";
        return 1;
    }
    klee::InstructionInfoTable infoTable(M.get());

    maybe_print_statistics(M.get(), "Statistics before ");

    // remove unused from module, we don't need that
    ModuleWriter writer(options, M.get());
    writer.removeUnusedFromModule();

    if (remove_unused_only) {
        errs() << "[llvm-slicer] removed unused parts of module, exiting...\n";
        maybe_print_statistics(M.get(), "Statistics after ");
        return writer.saveModule(should_verify_module);
    }

    /// ---------------
    // slice the code
    /// ---------------
    if (options.cutoffDiverging && options.dgOptions.threads) {
        llvm::errs() << "[llvm-slicer] threads are enabled, not cutting off "
                        "diverging\n";
        options.cutoffDiverging = false;
    }

    if (options.cutoffDiverging) {
        DBG(llvm - slicer, "Searching for slicing criteria values");
        auto csvalues = getSlicingCriteriaValues(
                *M, options.slicingCriteria, options.legacySlicingCriteria,
                options.legacySecondarySlicingCriteria,
                options.criteriaAreNextInstr);
        if (csvalues.empty()) {
            llvm::errs() << "No reachable slicing criteria: '"
                         << options.slicingCriteria << "' '"
                         << options.legacySlicingCriteria << "'\n";
            ::Slicer slicer(M.get(), options);
            if (!slicer.createEmptyMain()) {
                llvm::errs() << "ERROR: failed creating an empty main\n";
                return 1;
            }

            maybe_print_statistics(M.get(), "Statistics after ");
            return writer.cleanAndSaveModule(should_verify_module);
        }

        DBG(llvm - slicer, "Cutting off diverging branches");
        if (!llvmdg::cutoffDivergingBranches(
                    *M, options.dgOptions.entryFunction, csvalues)) {
            errs() << "[llvm-slicer]: Failed cutting off diverging branches\n";
            return 1;
        }

        maybe_print_statistics(M.get(), "Statistics after cutoff-diverging ");
    }

    ::Slicer slicer(M.get(), options);
    if (!slicer.buildDG()) {
        errs() << "ERROR: Failed building DG\n";
        return 1;
    }

    ModuleAnnotator annotator(options, &slicer.getDG(),
                              parseAnnotationOptions(annotationOpts));

    std::set<LLVMNode *> criteria_nodes;
    if (!getSlicingCriteriaNodes(slicer.getDG(), options.slicingCriteria,
                                 options.legacySlicingCriteria,
                                 options.legacySecondarySlicingCriteria,
                                 criteria_nodes,
                                 options.criteriaAreNextInstr)) {
        llvm::errs() << "ERROR: Failed finding slicing criteria: '"
                     << options.slicingCriteria << "'\n";

        if (annotator.shouldAnnotate()) {
            slicer.computeDependencies();
            annotator.annotate();
        }

        return 1;
    }

    if (criteria_nodes.empty()) {
        llvm::errs() << "No reachable slicing criteria: '"
                     << options.slicingCriteria << "' '"
                     << options.legacySlicingCriteria << "'\n";
        if (annotator.shouldAnnotate()) {
            slicer.computeDependencies();
            annotator.annotate();
        }

        if (!slicer.createEmptyMain()) {
            llvm::errs() << "ERROR: failed creating an empty main\n";
            return 1;
        }

        maybe_print_statistics(M.get(), "Statistics after ");
        return writer.cleanAndSaveModule(should_verify_module);
    }

    // mark nodes that are going to be in the slice
    if (!slicer.mark(criteria_nodes)) {
        llvm::errs() << "Finding dependent nodes failed\n";
        return 1;
    }

    // print debugging llvm IR if user asked for it
    if (annotator.shouldAnnotate())
        annotator.annotate(&criteria_nodes);

    DGDumper dumper(options, &slicer.getDG(), dump_bb_only);
    if (dump_dg) {
        dumper.dumpToDot();

        if (dump_dg_only)
            return 0;
    }

    // slice the graph
    std::vector<int> slicedLines = {};
    if (!slicer.slice(&infoTable, &slicedLines)) {
        errs() << "ERROR: Slicing failed\n";
        return 1;
    }
    #ifdef ENABLE_JSON
    SlicedJson sj(M.get());
    sj.build();
    std::string jsonOutput = options.inputFile;
    replace_suffix(jsonOutput, ".sliced.json");
    sj.saveOutput(jsonOutput);
    #endif
    if (dump_dg) {
        dumper.dumpToDot(".sliced.dot");
    }

    std::ofstream outfile;
    std::string outStr;
    // remove duplicate elements in slicedLines
    std::sort(slicedLines.begin(), slicedLines.end());
    slicedLines.erase(std::unique(slicedLines.begin(), slicedLines.end()),
                      slicedLines.end());

    outfile.open("line-infos.csv", std::ios::out);
    for (auto line : slicedLines) {
        outStr += std::to_string(line) + ",";
    }
    outStr.pop_back();
    outfile << outStr;
    outfile.close();

    // remove unused from module again, since slicing
    // could and probably did make some other parts unused
    maybe_print_statistics(M.get(), "Statistics after ");
    return writer.cleanAndSaveModule(should_verify_module);
}
