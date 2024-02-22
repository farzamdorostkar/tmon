//===- Transforms/Instrumentation/ThreadMonitor.cpp - TMon Pass -----------===//
//
// Author: Farzam Dorostkar
// Email:  farzam.dorostkar@polymtl.ca
// Lab:    DORSAL - Polytechnique Montreal
//
//===--------------------------------------------------------------------===//
//
// This file defines the ThreadMonitor (TMon) pass.
// This file was inspired by "ThreadSanitizer.cpp".
//
//===--------------------------------------------------------------------===//

#include "llvm/Transforms/Instrumentation/ThreadMonitor.h"
#include "llvm/IR/InlineAsm.h"
#include "ptwrite_instrumentation.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/EscapeEnumerator.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

using namespace llvm;

#define DEBUG_TYPE "tmon"

static cl::opt<bool> ClInstrumentMemoryAccesses(
    "tmon-instrument-memory-accesses", cl::init(true),
    cl::desc("Instrument memory accesses"), cl::Hidden);
static cl::opt<bool>
    ClInstrumentFuncEntryExit("tmon-instrument-func-entry-exit", cl::init(true),
                              cl::desc("Instrument function entry and exit"),
                              cl::Hidden);
static cl::opt<bool> ClHandleCxxExceptions(
    "tmon-handle-cxx-exceptions", cl::init(true),
    cl::desc("Handle C++ exceptions (insert cleanup blocks for unwinding)"),
    cl::Hidden);
static cl::opt<bool> ClInstrumentAtomics("tmon-instrument-atomics",
                                         cl::init(true),
                                         cl::desc("Instrument atomics"),
                                         cl::Hidden);
static cl::opt<bool> ClInstrumentMemIntrinsics(
    "tmon-instrument-memintrinsics", cl::init(true),
    cl::desc("Instrument memintrinsics (memset/memcpy/memmove)"), cl::Hidden);
static cl::opt<bool> ClDistinguishVolatile(
    "tmon-distinguish-volatile", cl::init(false),
    cl::desc("Emit special instrumentation for accesses to volatiles"),
    cl::Hidden);
static cl::opt<bool> ClInstrumentReadBeforeWrite(
    "tmon-instrument-read-before-write", cl::init(false),
    cl::desc("Do not eliminate read instrumentation for read-before-writes"),
    cl::Hidden);
static cl::opt<bool> ClCompoundReadBeforeWrite(
    "tmon-compound-read-before-write", cl::init(false),
    cl::desc("Emit special compound instrumentation for reads-before-writes"),
    cl::Hidden);

STATISTIC(NumInstrumentedReads, "Number of instrumented reads");
STATISTIC(NumInstrumentedWrites, "Number of instrumented writes");
STATISTIC(NumOmittedReadsBeforeWrite,
          "Number of reads ignored due to following writes");
STATISTIC(NumAccessesWithBadSize, "Number of accesses with bad size");
STATISTIC(NumInstrumentedVtableWrites, "Number of vtable ptr writes");
STATISTIC(NumInstrumentedVtableReads, "Number of vtable ptr reads");
STATISTIC(NumOmittedReadsFromConstantGlobals,
          "Number of reads from constant globals");
STATISTIC(NumOmittedReadsFromVtable, "Number of vtable reads");
STATISTIC(NumOmittedNonCaptured, "Number of accesses ignored due to capturing");

const char kTsanModuleCtorName[] = "tsan.module_ctor";
const char kTsanInitName[] = "__tsan_init";

namespace {

/// ThreadMonitor: instrument the code in module to find races.
///
/// Instantiating ThreadMonitor inserts the tsan runtime library API function
/// declarations into the module if they don't exist already. Instantiating
/// ensures the __tsan_init function is in the list of global constructors for
/// the module.
struct ThreadMonitor {
  ThreadMonitor() {
    // Check options and warn user.
    if (ClInstrumentReadBeforeWrite && ClCompoundReadBeforeWrite) {
      errs()
          << "warning: Option -tmon-compound-read-before-write has no effect "
             "when -tmon-instrument-read-before-write is set.\n";
    }
  }

  bool sanitizeFunction(Function &F, const TargetLibraryInfo &TLI);

private:
  // Internal Instruction wrapper that contains more information about the
  // Instruction from prior analysis.
  struct InstructionInfo {
    // Instrumentation emitted for this instruction is for a compounded set of
    // read and write operations in the same basic block.
    static constexpr unsigned kCompoundRW = (1U << 0);

    explicit InstructionInfo(Instruction *Inst) : Inst(Inst) {}

    Instruction *Inst;
    unsigned Flags = 0;
  };

  void initialize(Module &M);
  bool instrumentLoadOrStore(const InstructionInfo &II, const DataLayout &DL);
  bool instrumentAtomic(Instruction *I, const DataLayout &DL);
  bool instrumentMemIntrinsic(Instruction *I);
  void chooseInstructionsToInstrument(SmallVectorImpl<Instruction *> &Local,
                                      SmallVectorImpl<InstructionInfo> &All,
                                      const DataLayout &DL);
  bool addrPointsToConstantData(Value *Addr);
  int getMemoryAccessFuncIndex(Type *OrigTy, Value *Addr, const DataLayout &DL);
  void InsertRuntimeIgnores(Function &F);

  // -------------------------------------------------------------------
  // [farzam] my code - begin ------------------------------------------
  void addAttribute(Function &F);

  /*void instrumentPthreadCreate(CallInst *CI);
  void instrumentPthreadJoin(CallInst *CI);
  void instrumentPthreadMutexLock(CallInst *CI);
  void instrumentPthreadMutexUnlock(CallInst *CI);*/
  // [farzam] my code - end --------------------------------------------
  // -------------------------------------------------------------------

  Type *IntptrTy;
  ///FunctionCallee TsanFuncEntry;                                
  ///FunctionCallee TsanFuncExit;                                   
  ///FunctionCallee TsanIgnoreBegin;                                 
  ///FunctionCallee TsanIgnoreEnd;                                   
  // Accesses sizes are powers of two: 1, 2, 4, 8, 16.
  static const size_t kNumberOfAccessSizes = 5;
  ///FunctionCallee TsanRead[kNumberOfAccessSizes];                  
  ///FunctionCallee TsanWrite[kNumberOfAccessSizes];                
  ///FunctionCallee TsanUnalignedRead[kNumberOfAccessSizes];         
  ///FunctionCallee TsanUnalignedWrite[kNumberOfAccessSizes];  
  ///FunctionCallee TsanVolatileRead[kNumberOfAccessSizes];      
  ///FunctionCallee TsanVolatileWrite[kNumberOfAccessSizes];        
  ///FunctionCallee TsanUnalignedVolatileRead[kNumberOfAccessSizes]; 
  ///FunctionCallee TsanUnalignedVolatileWrite[kNumberOfAccessSizes];
  ///FunctionCallee TsanCompoundRW[kNumberOfAccessSizes];          
  ///FunctionCallee TsanUnalignedCompoundRW[kNumberOfAccessSizes];  
  //FunctionCallee TsanAtomicLoad[kNumberOfAccessSizes];         
  //FunctionCallee TsanAtomicStore[kNumberOfAccessSizes];
  /*FunctionCallee TsanAtomicRMW[AtomicRMWInst::LAST_BINOP + 1]
                              [kNumberOfAccessSizes];*/
  //FunctionCallee TsanAtomicCAS[kNumberOfAccessSizes];
  //FunctionCallee TsanAtomicThreadFence;
  //FunctionCallee TsanAtomicSignalFence;
  ///FunctionCallee TsanVptrUpdate;               
  ///FunctionCallee TsanVptrLoad;                
  FunctionCallee MemmoveFn, MemcpyFn, MemsetFn;

  FunctionCallee PthreadSelf;

  //FunctionType *FTyAtomicLoad[kNumberOfAccessSizes][2];

  static constexpr uint64_t TMonFuncEntry = PTW_FUNC_ENTRY_MASK;
  
  static constexpr uint64_t TMonReadMask[] = {
    PTW_READ1_MASK,
    PTW_READ2_MASK, 
    PTW_READ4_MASK, 
    PTW_READ8_MASK, 
    PTW_READ16_MASK
  };

  static constexpr uint64_t TMonWriteMask[] = {
    PTW_WRITE1_MASK,
    PTW_WRITE2_MASK,
    PTW_WRITE4_MASK,
    PTW_WRITE8_MASK,
    PTW_WRITE16_MASK
  };

  static constexpr uint64_t TMonUnalignedReadMask[] = {
    PTW_UNALIGNED_READ1_MASK,
    PTW_UNALIGNED_READ2_MASK,
    PTW_UNALIGNED_READ4_MASK,
    PTW_UNALIGNED_READ8_MASK,
    PTW_UNALIGNED_READ16_MASK
  };

  static constexpr uint64_t TMonUnalignedWriteMask[] = {
    PTW_UNALIGNED_WRITE1_MASK,
    PTW_UNALIGNED_WRITE2_MASK,
    PTW_UNALIGNED_WRITE4_MASK,
    PTW_UNALIGNED_WRITE8_MASK,
    PTW_UNALIGNED_WRITE16_MASK
  };
  
  static constexpr uint64_t TMonReadWriteMask[] = {
    PTW_READ_WRITE1_MASK,
    PTW_READ_WRITE2_MASK, 
    PTW_READ_WRITE4_MASK,
    PTW_READ_WRITE8_MASK,
    PTW_READ_WRITE16_MASK
  };
  
  static constexpr uint64_t TMonVolatileReadMask[] = {
    PTW_VOLATILE_READ1_MASK,
    PTW_VOLATILE_READ2_MASK,
    PTW_VOLATILE_READ4_MASK,
    PTW_VOLATILE_READ8_MASK,
    PTW_VOLATILE_READ16_MASK
  };
  
  static constexpr uint64_t TMonVolatileWriteMask[] = {
    PTW_VOLATILE_WRITE1_MASK,
    PTW_VOLATILE_WRITE2_MASK,
    PTW_VOLATILE_WRITE4_MASK,
    PTW_VOLATILE_WRITE8_MASK,
    PTW_VOLATILE_WRITE16_MASK
  };
  
  static constexpr uint64_t TMonUnalignedReadWriteMask[] = {
    PTW_UNALIGNED_READ_WRITE1_MASK,
    PTW_UNALIGNED_READ_WRITE2_MASK,
    PTW_UNALIGNED_READ_WRITE4_MASK,
    PTW_UNALIGNED_READ_WRITE8_MASK,
    PTW_UNALIGNED_READ_WRITE16_MASK
  };
  
  static constexpr uint64_t TMonUnalignedVolatileReadMask[] = {
    PTW_UNALIGNED_VOLATILE_READ1_MASK,
    PTW_UNALIGNED_VOLATILE_READ2_MASK,
    PTW_UNALIGNED_VOLATILE_READ4_MASK,
    PTW_UNALIGNED_VOLATILE_READ8_MASK,
    PTW_UNALIGNED_VOLATILE_READ16_MASK
  };

  static constexpr uint64_t TMonUnalignedVolatileWriteMask[] = {
    PTW_UNALIGNED_VOLATILE_WRITE1_MASK,
    PTW_UNALIGNED_VOLATILE_WRITE2_MASK,
    PTW_UNALIGNED_VOLATILE_WRITE4_MASK,
    PTW_UNALIGNED_VOLATILE_WRITE8_MASK,
    PTW_UNALIGNED_VOLATILE_WRITE16_MASK
  };

  static constexpr uint64_t TMonAtomicLoadMask[] = {
    PTW_ATOMIC8_LOAD_MASK,
    PTW_ATOMIC16_LOAD_MASK,
    PTW_ATOMIC32_LOAD_MASK,
    PTW_ATOMIC64_LOAD_MASK,
    PTW_ATOMIC128_LOAD_MASK
  };

  static constexpr uint64_t TMonAtomicStoreMask[] = {
    PTW_ATOMIC8_STORE_MASK,
    PTW_ATOMIC16_STORE_MASK,
    PTW_ATOMIC32_STORE_MASK,
    PTW_ATOMIC64_STORE_MASK,
    PTW_ATOMIC128_STORE_MASK
  };

  static constexpr uint64_t TMonAtomicRMWMask[] = {
    PTW_ATOMIC8_RMW_MASK,
    PTW_ATOMIC16_RMW_MASK,
    PTW_ATOMIC32_RMW_MASK,
    PTW_ATOMIC64_RMW_MASK,
    PTW_ATOMIC128_RMW_MASK
  };

  static constexpr uint64_t TMonAtomicCmpXChgMask[] = {
    PTW_ATOMIC8_CMPXCHG_MASK,
    PTW_ATOMIC16_CMPXCHG_MASK,
    PTW_ATOMIC32_CMPXCHG_MASK,
    PTW_ATOMIC64_CMPXCHG_MASK,
    PTW_ATOMIC128_CMPXCHG_MASK
  };
};

void insertModuleCtor(Module &M) {  //farzam: inserts "tsan.module_ctor" in the binary?
  getOrCreateSanitizerCtorAndInitFunctions(
      M, kTsanModuleCtorName, kTsanInitName, /*InitArgTypes=*/{},
      /*InitArgs=*/{},
      // This callback is invoked when the functions are created the first
      // time. Hook them into the global ctors list in that case:
      [&](Function *Ctor, FunctionCallee) { appendToGlobalCtors(M, Ctor, 0); });
}
}  // namespace

PreservedAnalyses ThreadMonitorPass::run(Function &F,
                                           FunctionAnalysisManager &FAM) {
  ThreadMonitor TMon;
  if (TMon.sanitizeFunction(F, FAM.getResult<TargetLibraryAnalysis>(F)))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}

PreservedAnalyses ModuleThreadMonitorPass::run(Module &M,
                                                 ModuleAnalysisManager &MAM) {
  insertModuleCtor(M);
  return PreservedAnalyses::none();
}

void ThreadMonitor::initialize(Module &M) {
  const DataLayout &DL = M.getDataLayout();
  IntptrTy = DL.getIntPtrType(M.getContext());

  IRBuilder<> IRB(M.getContext());
  AttributeList Attr;
  Attr = Attr.addFnAttribute(M.getContext(), Attribute::NoUnwind);
  // Initialize the callbacks.
  // ******************** farzam: commented (begin) ********************
  /*TsanFuncEntry = M.getOrInsertFunction("__tsan_func_entry", Attr,
                                        IRB.getVoidTy(), IRB.getInt8PtrTy());/*
  /*TsanFuncExit =
      M.getOrInsertFunction("__tsan_func_exit", Attr, IRB.getVoidTy());*/

  /*TsanIgnoreBegin = M.getOrInsertFunction("__tsan_ignore_thread_begin", Attr,
                                          IRB.getVoidTy());*/
  /*TsanIgnoreEnd =
      M.getOrInsertFunction("__tsan_ignore_thread_end", Attr, IRB.getVoidTy());*/
  // ********************* farzam: commented (end) *********************

  // -------------------------------------------------------------------
  // [farzam] function declaration - begin -----------------------------
  //PthreadSelf = M.getOrInsertFunction("pthread_self", Attr, IRB.getInt64Ty());
  // [farzam] function declaration - end -------------------------------
  // -------------------------------------------------------------------

  //IntegerType *OrdTy = IRB.getInt32Ty();
  for (size_t i = 0; i < kNumberOfAccessSizes; ++i) {
    const unsigned ByteSize = 1U << i;
    const unsigned BitSize = ByteSize * 8;
    std::string ByteSizeStr = utostr(ByteSize);
    std::string BitSizeStr = utostr(BitSize);

    // ******************** farzam: commented code (begin) ********************
    /*SmallString<32> ReadName("__tsan_read" + ByteSizeStr);
    TsanRead[i] = M.getOrInsertFunction(ReadName, Attr, IRB.getVoidTy(),
                                        IRB.getInt8PtrTy());*/
    
    /*SmallString<32> WriteName("__tsan_write" + ByteSizeStr);  //farzam: creating "__tsan_write" function names
    TsanWrite[i] = M.getOrInsertFunction(WriteName, Attr, IRB.getVoidTy(),  //farzam: for instance TsanWrite[0] is the __tsan_write1 function
                                         IRB.getInt8PtrTy());*/
    
    /*SmallString<64> UnalignedReadName("__tsan_unaligned_read" + ByteSizeStr);
    TsanUnalignedRead[i] = M.getOrInsertFunction(
        UnalignedReadName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> UnalignedWriteName("__tsan_unaligned_write" + ByteSizeStr);
    TsanUnalignedWrite[i] = M.getOrInsertFunction(
        UnalignedWriteName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> VolatileReadName("__tsan_volatile_read" + ByteSizeStr);
    TsanVolatileRead[i] = M.getOrInsertFunction(
        VolatileReadName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> VolatileWriteName("__tsan_volatile_write" + ByteSizeStr);
    TsanVolatileWrite[i] = M.getOrInsertFunction(
        VolatileWriteName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> UnalignedVolatileReadName("__tsan_unaligned_volatile_read" +
                                              ByteSizeStr);
    TsanUnalignedVolatileRead[i] = M.getOrInsertFunction(
        UnalignedVolatileReadName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> UnalignedVolatileWriteName(
        "PTW_UNALIGNED_VOLATILE_WRITE" + ByteSizeStr);
    TsanUnalignedVolatileWrite[i] = M.getOrInsertFunction(
        UnalignedVolatileWriteName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> CompoundRWName("PTW_READ_WRITE" + ByteSizeStr);
    TsanCompoundRW[i] = M.getOrInsertFunction(
        CompoundRWName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/

    /*SmallString<64> UnalignedCompoundRWName("__tsan_unaligned_read_write" +
                                            ByteSizeStr);
    TsanUnalignedCompoundRW[i] = M.getOrInsertFunction(
        UnalignedCompoundRWName, Attr, IRB.getVoidTy(), IRB.getInt8PtrTy());*/
    // ********************* farzam: commented code (end) *********************

    Type *Ty = Type::getIntNTy(M.getContext(), BitSize);
    Type *PtrTy = Ty->getPointerTo();
    SmallString<32> AtomicLoadName("__tsan_atomic" + BitSizeStr + "_load");
    {
      AttributeList AL = Attr;
      AL = AL.addParamAttribute(M.getContext(), 1, Attribute::ZExt);
      /*TsanAtomicLoad[i] =
          M.getOrInsertFunction(AtomicLoadName, AL, Ty, PtrTy, OrdTy);*/    //farzam: ty is return type
    }

    SmallString<32> AtomicStoreName("__tsan_atomic" + BitSizeStr + "_store");
    {
      AttributeList AL = Attr;
      AL = AL.addParamAttribute(M.getContext(), 1, Attribute::ZExt);
      AL = AL.addParamAttribute(M.getContext(), 2, Attribute::ZExt);
      /*TsanAtomicStore[i] = M.getOrInsertFunction(
          AtomicStoreName, AL, IRB.getVoidTy(), PtrTy, Ty, OrdTy);*/
    }

    for (unsigned Op = AtomicRMWInst::FIRST_BINOP;
         Op <= AtomicRMWInst::LAST_BINOP; ++Op) {
      //TsanAtomicRMW[Op][i] = nullptr;
      const char *NamePart = nullptr;
      if (Op == AtomicRMWInst::Xchg)
        NamePart = "_exchange";
      else if (Op == AtomicRMWInst::Add)
        NamePart = "_fetch_add";
      else if (Op == AtomicRMWInst::Sub)
        NamePart = "_fetch_sub";
      else if (Op == AtomicRMWInst::And)
        NamePart = "_fetch_and";
      else if (Op == AtomicRMWInst::Or)
        NamePart = "_fetch_or";
      else if (Op == AtomicRMWInst::Xor)
        NamePart = "_fetch_xor";
      else if (Op == AtomicRMWInst::Nand)
        NamePart = "_fetch_nand";
      else
        continue;
      SmallString<32> RMWName("__tsan_atomic" + itostr(BitSize) + NamePart);
      {
        AttributeList AL = Attr;
        AL = AL.addParamAttribute(M.getContext(), 1, Attribute::ZExt);
        AL = AL.addParamAttribute(M.getContext(), 2, Attribute::ZExt);
        /*TsanAtomicRMW[Op][i] =
            M.getOrInsertFunction(RMWName, AL, Ty, PtrTy, Ty, OrdTy);*/
      }
    }

    SmallString<32> AtomicCASName("__tsan_atomic" + BitSizeStr +
                                  "_compare_exchange_val");
    {
      AttributeList AL = Attr;
      AL = AL.addParamAttribute(M.getContext(), 1, Attribute::ZExt);
      AL = AL.addParamAttribute(M.getContext(), 2, Attribute::ZExt);
      AL = AL.addParamAttribute(M.getContext(), 3, Attribute::ZExt);
      AL = AL.addParamAttribute(M.getContext(), 4, Attribute::ZExt);
      /*TsanAtomicCAS[i] = M.getOrInsertFunction(AtomicCASName, AL, Ty, PtrTy, Ty,
                                               Ty, OrdTy, OrdTy);*/
    }
  } //farzam: end of for loop of 5 access sizes
  /*TsanVptrUpdate =
      M.getOrInsertFunction("__tsan_vptr_update", Attr, IRB.getVoidTy(),
                            IRB.getInt8PtrTy(), IRB.getInt8PtrTy());*/  //farzam: takes two pointers
  /*TsanVptrLoad = M.getOrInsertFunction("__tsan_vptr_read", Attr,
                                       IRB.getVoidTy(), IRB.getInt8PtrTy());*/
  {
    AttributeList AL = Attr;
    AL = AL.addParamAttribute(M.getContext(), 0, Attribute::ZExt);
    /*TsanAtomicThreadFence = M.getOrInsertFunction("__tsan_atomic_thread_fence",
                                                  AL, IRB.getVoidTy(), OrdTy);*/
  }
  {
    AttributeList AL = Attr;
    AL = AL.addParamAttribute(M.getContext(), 0, Attribute::ZExt);
    /*TsanAtomicSignalFence = M.getOrInsertFunction("__tsan_atomic_signal_fence",
                                                  AL, IRB.getVoidTy(), OrdTy);*/
  }

  MemmoveFn =
      M.getOrInsertFunction("memmove", Attr, IRB.getInt8PtrTy(),
                            IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IntptrTy);
  MemcpyFn =
      M.getOrInsertFunction("memcpy", Attr, IRB.getInt8PtrTy(),
                            IRB.getInt8PtrTy(), IRB.getInt8PtrTy(), IntptrTy);
  MemsetFn =
      M.getOrInsertFunction("memset", Attr, IRB.getInt8PtrTy(),
                            IRB.getInt8PtrTy(), IRB.getInt32Ty(), IntptrTy);
}

static bool isVtableAccess(Instruction *I) {
  if (MDNode *Tag = I->getMetadata(LLVMContext::MD_tbaa))
    return Tag->isTBAAVtableAccess();
  return false;
}

// Do not instrument known races/"benign races" that come from compiler
// instrumentatin. The user has no way of suppressing them.
static bool shouldInstrumentReadWriteFromAddress(const Module *M, Value *Addr) {
  // Peel off GEPs and BitCasts.
  Addr = Addr->stripInBoundsOffsets();

  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Addr)) {
    if (GV->hasSection()) {
      StringRef SectionName = GV->getSection();
      // Check if the global is in the PGO counters section.
      auto OF = Triple(M->getTargetTriple()).getObjectFormat();
      if (SectionName.endswith(
              getInstrProfSectionName(IPSK_cnts, OF, /*AddSegmentInfo=*/false)))
        return false;
    }

    // Check if the global is private gcov data.
    if (GV->getName().startswith("__llvm_gcov") ||
        GV->getName().startswith("__llvm_gcda"))
      return false;
  }

  // Do not instrument accesses from different address spaces; we cannot deal
  // with them.
  if (Addr) {
    Type *PtrTy = cast<PointerType>(Addr->getType()->getScalarType());
    if (PtrTy->getPointerAddressSpace() != 0)
      return false;
  }

  return true;
}

bool ThreadMonitor::addrPointsToConstantData(Value *Addr) {
  // If this is a GEP, just analyze its pointer operand.
  if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Addr))
    Addr = GEP->getPointerOperand();

  if (GlobalVariable *GV = dyn_cast<GlobalVariable>(Addr)) {
    if (GV->isConstant()) {
      // Reads from constant globals can not race with any writes.
      NumOmittedReadsFromConstantGlobals++;
      return true;
    }
  } else if (LoadInst *L = dyn_cast<LoadInst>(Addr)) {
    if (isVtableAccess(L)) {
      // Reads from a vtable pointer can not race with any writes.
      NumOmittedReadsFromVtable++;
      return true;
    }
  }
  return false;
}

// Instrumenting some of the accesses may be proven redundant.
// Currently handled:
//  - read-before-write (within same BB, no calls between)
//  - not captured variables
//
// We do not handle some of the patterns that should not survive
// after the classic compiler optimizations.
// E.g. two reads from the same temp should be eliminated by CSE,
// two writes should be eliminated by DSE, etc.
//
// 'Local' is a vector of insns within the same BB (no calls between).
// 'All' is a vector of insns that will be instrumented.
void ThreadMonitor::chooseInstructionsToInstrument(
    SmallVectorImpl<Instruction *> &Local,
    SmallVectorImpl<InstructionInfo> &All, const DataLayout &DL) {
  DenseMap<Value *, size_t> WriteTargets; // Map of addresses to index in All
  // Iterate from the end.
  for (Instruction *I : reverse(Local)) {
    const bool IsWrite = isa<StoreInst>(*I);
    Value *Addr = IsWrite ? cast<StoreInst>(I)->getPointerOperand()
                          : cast<LoadInst>(I)->getPointerOperand();

    if (!shouldInstrumentReadWriteFromAddress(I->getModule(), Addr))
      continue;

    if (!IsWrite) {
      const auto WriteEntry = WriteTargets.find(Addr);
      if (!ClInstrumentReadBeforeWrite && WriteEntry != WriteTargets.end()) {
        auto &WI = All[WriteEntry->second];
        // If we distinguish volatile accesses and if either the read or write
        // is volatile, do not omit any instrumentation.
        const bool AnyVolatile =
            ClDistinguishVolatile && (cast<LoadInst>(I)->isVolatile() ||
                                      cast<StoreInst>(WI.Inst)->isVolatile());
        if (!AnyVolatile) {
          // We will write to this temp, so no reason to analyze the read.
          // Mark the write instruction as compound.
          WI.Flags |= InstructionInfo::kCompoundRW;
          NumOmittedReadsBeforeWrite++;
          continue;
        }
      }

      if (addrPointsToConstantData(Addr)) {
        // Addr points to some constant data -- it can not race with any writes.
        continue;
      }
    }

    if (isa<AllocaInst>(getUnderlyingObject(Addr)) &&
        !PointerMayBeCaptured(Addr, true, true)) {
      // The variable is addressable but not captured, so it cannot be
      // referenced from a different thread and participate in a data race
      // (see llvm/Analysis/CaptureTracking.h for details).
      NumOmittedNonCaptured++;
      continue;
    }

    // Instrument this instruction.
    All.emplace_back(I);
    if (IsWrite) {
      // For read-before-write and compound instrumentation we only need one
      // write target, and we can override any previous entry if it exists.
      WriteTargets[Addr] = All.size() - 1;
    }
  }
  Local.clear();
}

static bool isTsanAtomic(const Instruction *I) {
  // TODO: Ask TTI whether synchronization scope is between threads.
  auto SSID = getAtomicSyncScopeID(I);
  if (!SSID)
    return false;
  if (isa<LoadInst>(I) || isa<StoreInst>(I))
    return SSID.value() != SyncScope::SingleThread;
  return true;
}

void ThreadMonitor::InsertRuntimeIgnores(Function &F) {
  InstrumentationIRBuilder IRB(F.getEntryBlock().getFirstNonPHI());

  // *************************************************************************
  // ******************** ptw ignore thread begin (begin) ********************
  {
    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);

    ConstantInt *ptwl = ConstantInt::get(IRB.getInt64Ty(), PTW_IGNORE_THREAD_BEGIN_MASK, false);

    InlineAsm *asmIgnBeg = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {ptwl->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    
    IRB.CreateCall(asmIgnBeg, {ptwl})->setAttributes(Attr); //TODO: not tested

    /*errs() << "ignore begin \n";
    errs() << "ptwl is of type: \n";
    ptwl->getType()->dump();*/
  }
  // ********************* ptw ignore thread begin (end) *********************
  // *************************************************************************

  // ******************** farzam: replaced code ********************
  //IRB.CreateCall(TsanIgnoreBegin);

  EscapeEnumerator EE(F, "tsan_ignore_cleanup", ClHandleCxxExceptions);
  while (IRBuilder<> *AtExit = EE.Next()) {
    InstrumentationIRBuilder::ensureDebugInfo(*AtExit, F);

    // ***********************************************************************
    // ******************** ptw ignore thread end (begin) ********************
    {
      AttributeList Attr;
      Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);

      ConstantInt *ptwl = ConstantInt::get(IRB.getInt64Ty(), PTW_IGNORE_THREAD_END_MASK, false);

      InlineAsm *asmIgnEnd = InlineAsm::get(
          FunctionType::get(IRB.getVoidTy(), {ptwl->getType()} , false), 
          StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
      
      IRB.CreateCall(asmIgnEnd, {ptwl})->setAttributes(Attr); //TODO: not tested

      /*errs() << "ignore end \n";
      errs() << "ptwl is of type: \n";
      ptwl->getType()->dump();*/
    }
    // ********************* ptw ignore thread end (end) *********************
    // ***********************************************************************

    // ******************** farzam: replaced code ********************
    //AtExit->CreateCall(TsanIgnoreEnd);
  }
}

bool ThreadMonitor::sanitizeFunction(Function &F,
                                       const TargetLibraryInfo &TLI) {
  // This is required to prevent instrumenting call to __tsan_init from within
  // the module constructor.
  if (F.getName() == kTsanModuleCtorName)
    return false;

  // Naked functions can not have prologue/epilogue
  // (TMonFuncEntry/TMonFuncExit) generated, so don't instrument them at
  // all.
  if (F.hasFnAttribute(Attribute::Naked))
    return false;

  // __attribute__(disable_sanitizer_instrumentation) prevents all kinds of
  // instrumentation.
  if (F.hasFnAttribute(Attribute::DisableSanitizerInstrumentation))
    return false;
  
  // [farzam] my code - begin ------------------
  addAttribute(F); //adds Attribute::SanitizeThread
  // [farzam] my code - end --------------------

  initialize(*F.getParent());
  SmallVector<InstructionInfo, 8> AllLoadsAndStores;
  SmallVector<Instruction*, 8> LocalLoadsAndStores;
  SmallVector<Instruction*, 8> AtomicAccesses;
  SmallVector<Instruction*, 8> MemIntrinCalls;
  bool Res = false;
  bool HasCalls = false;
  bool SanitizeFunction = F.hasFnAttribute(Attribute::SanitizeThread);
  const DataLayout &DL = F.getParent()->getDataLayout();

  // Traverse all instructions, collect loads/stores/returns, check for calls.
  for (auto &BB : F) {
    for (auto &Inst : BB) {
      if (isTsanAtomic(&Inst))
        AtomicAccesses.push_back(&Inst);
      else if (isa<LoadInst>(Inst) || isa<StoreInst>(Inst)){
        LocalLoadsAndStores.push_back(&Inst);
      }
      else if ((isa<CallInst>(Inst) && !isa<DbgInfoIntrinsic>(Inst)) ||
               isa<InvokeInst>(Inst)) {
        if (CallInst *CI = dyn_cast<CallInst>(&Inst))
          maybeMarkSanitizerLibraryCallNoBuiltin(CI, &TLI);
        if (isa<MemIntrinsic>(Inst))
          MemIntrinCalls.push_back(&Inst);
        HasCalls = true;
        chooseInstructionsToInstrument(LocalLoadsAndStores, AllLoadsAndStores,
                                       DL);
      }
    }
    chooseInstructionsToInstrument(LocalLoadsAndStores, AllLoadsAndStores, DL);
  }

  // We have collected all loads and stores.
  // FIXME: many of these accesses do not need to be checked for races
  // (e.g. variables that do not escape, etc).

  // Instrument memory accesses only if we want to report bugs in the function.
  if (ClInstrumentMemoryAccesses && SanitizeFunction)
    for (const auto &II : AllLoadsAndStores) {
      Res |= instrumentLoadOrStore(II, DL);
    }

  // Instrument atomic memory accesses in any case (they can be used to
  // implement synchronization).
  if (ClInstrumentAtomics)
    for (auto *Inst : AtomicAccesses) {
      Res |= instrumentAtomic(Inst, DL);
    }

  if (ClInstrumentMemIntrinsics && SanitizeFunction)
    for (auto *Inst : MemIntrinCalls) {
      Res |= instrumentMemIntrinsic(Inst);
    }

  if (F.hasFnAttribute("sanitize_thread_no_checking_at_run_time")) {
    assert(!F.hasFnAttribute(Attribute::SanitizeThread));
    if (HasCalls)
      InsertRuntimeIgnores(F);
  }

  // Instrument function entry/exit points if there were instrumented accesses.
  if ((Res || HasCalls) && ClInstrumentFuncEntryExit) {
    InstrumentationIRBuilder IRB(F.getEntryBlock().getFirstNonPHI());

    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);

    Value *ReturnAddress = IRB.CreateCall(
        Intrinsic::getDeclaration(F.getParent(), Intrinsic::returnaddress),
        IRB.getInt32(0));
    Value *ReturnAddressInt = IRB.CreatePtrToInt(ReturnAddress, Type::getInt64Ty(IRB.getContext()));
    
    Value *PtwFuncEntry = IRB.CreateOr(ReturnAddressInt, TMonFuncEntry, "PtwFuncEntry");
    InlineAsm *asmFuncEntry = InlineAsm::get(
        FunctionType::get(Type::getVoidTy(IRB.getContext()), {PtwFuncEntry->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(asmFuncEntry, {PtwFuncEntry})->setAttributes(Attr);
    
    InlineAsm *asmFuncExit = InlineAsm::get(
        FunctionType::get(Type::getVoidTy(IRB.getContext()), {Type::getInt64Ty(IRB.getContext())}, false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    EscapeEnumerator EE(F, "tmon_cleanup", ClHandleCxxExceptions);
    while (IRBuilder<> *AtExit = EE.Next()) {
      InstrumentationIRBuilder::ensureDebugInfo(*AtExit, F);
      AtExit->CreateCall(asmFuncExit, {IRB.getInt64(FUNC_EXIT)})->setAttributes(Attr);
    }
    Res = true;
  }
  return Res;
}

bool ThreadMonitor::instrumentLoadOrStore(const InstructionInfo &II,
                                            const DataLayout &DL) {
  InstrumentationIRBuilder IRB(II.Inst);
  const bool IsWrite = isa<StoreInst>(*II.Inst);
  Value *Addr = IsWrite ? cast<StoreInst>(II.Inst)->getPointerOperand()
                        : cast<LoadInst>(II.Inst)->getPointerOperand();
  Type *OrigTy = getLoadStoreType(II.Inst);

  // swifterror memory addresses are mem2reg promoted by instruction selection.
  // As such they cannot have regular uses like an instrumentation function and
  // it makes no sense to track them as memory.
  if (Addr->isSwiftError())
    return false;

  int Idx = getMemoryAccessFuncIndex(OrigTy, Addr, DL);
  if (Idx < 0)
    return false;

  if (IsWrite && isVtableAccess(II.Inst)) { //TODO: [farzam] add the check from __tsan_vptr_update
    LLVM_DEBUG(dbgs() << "  VPTR : " << *II.Inst << "\n");

    Value *AddrInt = IRB.CreatePtrToInt(Addr, Type::getInt64Ty(IRB.getContext()));
    ConstantInt *MaskVal = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), PTW_VPTR_WRITE_MASK);
    Value *Temp = IRB.CreateOr(AddrInt, MaskVal);

    uint64_t PtrSize = DL.getPointerSize();
    ConstantInt *PtrSizeVal = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), PtrSize);
    ConstantInt *ShiftAmount = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), 56);
    Value *ShiftedSizeVal = IRB.CreateShl(PtrSizeVal, ShiftAmount);

    Value* PtwVptrWrite = IRB.CreateOr(Temp, ShiftedSizeVal, "PtwVptrWr");

    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
    InlineAsm *AsmVptrWrite = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {PtwVptrWrite->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(AsmVptrWrite, {PtwVptrWrite})->setAttributes(Attr);

    NumInstrumentedVtableWrites++;
    return true;
  }
  if (!IsWrite && isVtableAccess(II.Inst)) {
    Value *AddrInt = IRB.CreatePtrToInt(Addr, Type::getInt64Ty(IRB.getContext()));
    ConstantInt *MaskVal = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), PTW_VPTR_READ_MASK);
    Value *Temp = IRB.CreateOr(AddrInt, MaskVal);

    uint64_t PtrSize = DL.getPointerSize();
    ConstantInt *PtrSizeVal = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), PtrSize);
    ConstantInt *ShiftAmount = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), 56);
    Value *ShiftedSizeVal = IRB.CreateShl(PtrSizeVal, ShiftAmount);

    Value* PtwVptrRead = IRB.CreateOr(Temp, ShiftedSizeVal, "PtwVptrRd");
    
    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
    InlineAsm *AsmVptrRead = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {PtwVptrRead->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(AsmVptrRead, {PtwVptrRead})->setAttributes(Attr);
    
    NumInstrumentedVtableReads++;
    return true;
  }

  const Align Alignment = IsWrite ? cast<StoreInst>(II.Inst)->getAlign()
                                  : cast<LoadInst>(II.Inst)->getAlign();
  const bool IsCompoundRW =
      ClCompoundReadBeforeWrite && (II.Flags & InstructionInfo::kCompoundRW);
  const bool IsVolatile = ClDistinguishVolatile &&
                          (IsWrite ? cast<StoreInst>(II.Inst)->isVolatile()
                                   : cast<LoadInst>(II.Inst)->isVolatile());
  assert((!IsVolatile || !IsCompoundRW) && "Compound volatile invalid!");

  const uint32_t TypeSize = DL.getTypeStoreSizeInBits(OrigTy);

  ConstantInt *AccMaskVal = nullptr;
  StringRef ResName;

  if (Alignment >= Align(8) || (Alignment.value() % (TypeSize / 8)) == 0) {
    if (IsCompoundRW) {
      AccMaskVal = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonReadWriteMask[Idx]);
      ResName = "PtwRdWr";
    }
    else if (IsVolatile) {
      AccMaskVal = IsWrite ?
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonVolatileWriteMask[Idx]) :
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonVolatileReadMask[Idx]);
      ResName = IsWrite ? "PtwVolWr" : "PtwVolRd";
    }
    else {
      AccMaskVal = IsWrite ?
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonWriteMask[Idx]) :
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonReadMask[Idx]);
      ResName = IsWrite ? "PtwWr" : "PtwRd";
    }
  } else {
    if (IsCompoundRW) {
      AccMaskVal = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonUnalignedReadWriteMask[Idx]);
      ResName = "PtwUnalgnRdWr";
    }
    else if (IsVolatile) {
      AccMaskVal = IsWrite ?
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonUnalignedVolatileWriteMask[Idx]) :
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonUnalignedVolatileReadMask[Idx]);
      ResName = IsWrite ? "PtwUnalgnVolWr" : "PtwUnalgnVolRd";
    }
    else{
      AccMaskVal = IsWrite ?
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonUnalignedWriteMask[Idx]) :
        ConstantInt::get(Type::getInt64Ty(IRB.getContext()), TMonUnalignedReadMask[Idx]);
      ResName = IsWrite ? "PtwUnalgnWr" : "PtwUnalgnRd";
    }
  }

  Value *AddrInt = IRB.CreatePtrToInt(Addr, Type::getInt64Ty(IRB.getContext()));
  Value* PtwAcc = IRB.CreateOr(AddrInt, AccMaskVal, ResName);
    
  AttributeList Attr;
  Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
  InlineAsm *AsmAcc = InlineAsm::get(
      FunctionType::get(IRB.getVoidTy(), {PtwAcc->getType()} , false), 
      StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
  IRB.CreateCall(AsmAcc, {PtwAcc})->setAttributes(Attr);

  if (IsCompoundRW || IsWrite)
    NumInstrumentedWrites++;
  if (IsCompoundRW || !IsWrite)
    NumInstrumentedReads++;
  return true;
}

static ConstantInt *createOrdering(IRBuilder<> *IRB, AtomicOrdering ord) {
  uint32_t v = 0;
  switch (ord) {
    case AtomicOrdering::NotAtomic:
      llvm_unreachable("unexpected atomic ordering!");
    case AtomicOrdering::Unordered:              [[fallthrough]];
    case AtomicOrdering::Monotonic:              v = 0; break;
    // Not specified yet:
    // case AtomicOrdering::Consume:                v = 1; break;
    case AtomicOrdering::Acquire:                v = 2; break;
    case AtomicOrdering::Release:                v = 3; break;
    case AtomicOrdering::AcquireRelease:         v = 4; break;
    case AtomicOrdering::SequentiallyConsistent: v = 5; break;
  }
  return IRB->getInt32(v);
}

// If a memset intrinsic gets inlined by the code gen, we will miss races on it.
// So, we either need to ensure the intrinsic is not inlined, or instrument it.
// We do not instrument memset/memmove/memcpy intrinsics (too complicated),
// instead we simply replace them with regular function calls, which are then
// intercepted by the run-time.
// Since tmon is running after everyone else, the calls should not be
// replaced back with intrinsics. If that becomes wrong at some point,
// we will need to call e.g. __tsan_memset to avoid the intrinsics.
bool ThreadMonitor::instrumentMemIntrinsic(Instruction *I) { //TODO: [farzam] Check my interceptors for these three functions.
  IRBuilder<> IRB(I);
  if (MemSetInst *M = dyn_cast<MemSetInst>(I)) {
    IRB.CreateCall(
        MemsetFn,
        {IRB.CreatePointerCast(M->getArgOperand(0), IRB.getInt8PtrTy()),
         IRB.CreateIntCast(M->getArgOperand(1), IRB.getInt32Ty(), false),
         IRB.CreateIntCast(M->getArgOperand(2), IntptrTy, false)});
    I->eraseFromParent();
  } else if (MemTransferInst *M = dyn_cast<MemTransferInst>(I)) {
    IRB.CreateCall(
        isa<MemCpyInst>(M) ? MemcpyFn : MemmoveFn,
        {IRB.CreatePointerCast(M->getArgOperand(0), IRB.getInt8PtrTy()),
         IRB.CreatePointerCast(M->getArgOperand(1), IRB.getInt8PtrTy()),
         IRB.CreateIntCast(M->getArgOperand(2), IntptrTy, false)});
    I->eraseFromParent();
  }
  return false;
}

// All llvm, ThreadSanitizer, and ThreadMonitor atomic operations are based on
//  C++11/C1x standards. For background see C++11 standard.  A slightly older,
// publicly available draft of the standard (not entirely up-to-date, but close
// enough for casual browsing) is available here:
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3242.pdf
// The following page contains more background information:
// http://www.hpl.hp.com/personal/Hans_Boehm/c++mm/

bool ThreadMonitor::instrumentAtomic(Instruction *I, const DataLayout &DL) { //TODO: [farzam] double check
  InstrumentationIRBuilder IRB(I);
  if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
    Value *Addr = LI->getPointerOperand();
    Type *OrigTy = LI->getType();
    int Idx = getMemoryAccessFuncIndex(OrigTy, Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    Type *Ty = Type::getIntNTy(IRB.getContext(), BitSize);
    Type *PtrTy = Ty->getPointerTo();
    Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     createOrdering(&IRB, LI->getOrdering())};

    Value *Arg0 = IRB.CreatePtrToInt(Args[0], Type::getInt64Ty(IRB.getContext()));
    Value *OrArg0 = IRB.CreateOr(Arg0, TMonAtomicLoadMask[Idx]);
    Value *ZExArg1 = IRB.CreateZExt(Args[1], Type::getInt64Ty(IRB.getContext()));
    ConstantInt *ShiftAmount = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), 48);
    Value *ShArg1 = IRB.CreateShl(ZExArg1, ShiftAmount);
    Value *PL = IRB.CreateOr(OrArg0, ShArg1, "PtwAtmLd");

    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
    InlineAsm *IAsm = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {PL->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(IAsm, {PL})->setAttributes(Attr);
  } else if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
    Value *Addr = SI->getPointerOperand();
    int Idx =
        getMemoryAccessFuncIndex(SI->getValueOperand()->getType(), Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    Type *Ty = Type::getIntNTy(IRB.getContext(), BitSize);
    Type *PtrTy = Ty->getPointerTo();
    Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     createOrdering(&IRB, SI->getOrdering())};
    
    Value *Arg0 = IRB.CreatePtrToInt(Args[0], Type::getInt64Ty(IRB.getContext()));
    Value *OrArg0 = IRB.CreateOr(Arg0, TMonAtomicStoreMask[Idx]);
    Value *ZExArg1 = IRB.CreateZExt(Args[1], Type::getInt64Ty(IRB.getContext()));
    ConstantInt *ShiftAmount = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), 48);
    Value *ShArg1 = IRB.CreateShl(ZExArg1, ShiftAmount);
    Value *PL = IRB.CreateOr(OrArg0, ShArg1, "PtwAtmSt");

    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
    InlineAsm *IAsm = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {PL->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(IAsm, {PL})->setAttributes(Attr);
  } else if (AtomicRMWInst *RMWI = dyn_cast<AtomicRMWInst>(I)) {
    Value *Addr = RMWI->getPointerOperand();
    int Idx =
        getMemoryAccessFuncIndex(RMWI->getValOperand()->getType(), Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    Type *Ty = Type::getIntNTy(IRB.getContext(), BitSize);
    Type *PtrTy = Ty->getPointerTo();
    Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     createOrdering(&IRB, RMWI->getOrdering())};
    
    Value *Arg0 = IRB.CreatePtrToInt(Args[0], Type::getInt64Ty(IRB.getContext()));
    Value *OrArg0 = IRB.CreateOr(Arg0, TMonAtomicRMWMask[Idx]);
    Value *ZExArg1 = IRB.CreateZExt(Args[1], Type::getInt64Ty(IRB.getContext()));
    ConstantInt *ShiftAmount = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), 48);
    Value *ShArg1 = IRB.CreateShl(ZExArg1, ShiftAmount);
    Value *PL = IRB.CreateOr(OrArg0, ShArg1, "PtwAtmRMW");

    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
    InlineAsm *IAsm = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {PL->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(IAsm, {PL})->setAttributes(Attr);
  } else if (AtomicCmpXchgInst *CASI = dyn_cast<AtomicCmpXchgInst>(I)) {
    Value *Addr = CASI->getPointerOperand();
    Type *OrigOldValTy = CASI->getNewValOperand()->getType();
    int Idx = getMemoryAccessFuncIndex(OrigOldValTy, Addr, DL);
    if (Idx < 0)
      return false;
    const unsigned ByteSize = 1U << Idx;
    const unsigned BitSize = ByteSize * 8;
    Type *Ty = Type::getIntNTy(IRB.getContext(), BitSize);
    Type *PtrTy = Ty->getPointerTo();
    Value *Args[] = {IRB.CreatePointerCast(Addr, PtrTy),
                     createOrdering(&IRB, CASI->getSuccessOrdering())};
    
    Value *Arg0 = IRB.CreatePtrToInt(Args[0], Type::getInt64Ty(IRB.getContext()));
    Value *OrArg0 = IRB.CreateOr(Arg0, TMonAtomicCmpXChgMask[Idx]);
    Value *ZExArg1 = IRB.CreateZExt(Args[1], Type::getInt64Ty(IRB.getContext()));
    ConstantInt *ShiftAmount = ConstantInt::get(Type::getInt64Ty(IRB.getContext()), 48);
    Value *ShArg1 = IRB.CreateShl(ZExArg1, ShiftAmount);
    Value *PL = IRB.CreateOr(OrArg0, ShArg1, "PtwAtmCmpXChg");

    AttributeList Attr;
    Attr = Attr.addFnAttribute(IRB.getContext(), Attribute::NoUnwind);
    InlineAsm *IAsm = InlineAsm::get(
        FunctionType::get(IRB.getVoidTy(), {PL->getType()} , false), 
        StringRef("ptwriteq $0"), StringRef("rm"), false, false, InlineAsm::AD_ATT, false);
    IRB.CreateCall(IAsm, {PL})->setAttributes(Attr);
  }
  return true;
}

int ThreadMonitor::getMemoryAccessFuncIndex(Type *OrigTy, Value *Addr,
                                              const DataLayout &DL) {
  assert(OrigTy->isSized());
  uint32_t TypeSize = DL.getTypeStoreSizeInBits(OrigTy);
  if (TypeSize != 8  && TypeSize != 16 &&
      TypeSize != 32 && TypeSize != 64 && TypeSize != 128) {
    NumAccessesWithBadSize++;
    // Ignore all unusual sizes.
    return -1;
  }
  size_t Idx = llvm::countr_zero(TypeSize / 8);
  assert(Idx < kNumberOfAccessSizes);
  return Idx;
}

void ThreadMonitor::addAttribute(Function &F)
{
  F.addFnAttr(llvm::Attribute::SanitizeThread);
}
