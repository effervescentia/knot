open Kore;

let unit_tests = [
  Arg_ColorTest.suite,
  Arg_DebugTest.suite,
  Arg_EntryTest.suite,
  Arg_FailFastTest.suite,
  Arg_FixTest.suite,
  Arg_LogImportsTest.suite,
  Arg_OutDirTest.suite,
  Arg_PortTest.suite,
  Arg_RootDirTest.suite,
  Arg_SourceDirTest.suite,
  Arg_TargetTest.suite,
  BuildTest.suite,
  ConfigFileTest.suite,
  ReporterTest.suite,
  TaskTest.suite,
  Processor_BuildServeTest.suite,
  Processor_BuildTest.suite,
  Processor_BundleTest.suite,
  Processor_DevServeTest.suite,
  Processor_FormatTest.suite,
  Processor_LangServeTest.suite,
  Processor_LintTest.suite,
  Processor_WatchTest.suite,
  ProcessorTest.suite,
  UsageTest.suite,
];

let integration_tests = [FormatTest_IT.suite];
