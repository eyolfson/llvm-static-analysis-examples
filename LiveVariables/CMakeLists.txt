add_llvm_library(LLVMLiveVariables MODULE
  Pass.cpp

  PLUGIN_TOOL
  opt
)
