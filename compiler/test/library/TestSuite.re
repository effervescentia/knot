open Kore;

let tests =
  [
    FilenameTest.suite,
    ErrorTest.suite,
    FunctionalTest.suite,
    GraphTest.suite,
    HashtblTest.suite,
    InfixTest.suite,
    InputTest.suite,
    IntTest.suite,
    LazyStreamTest.suite,
    ListTest.suite,
    ModuleTableTest.suite,
    PointTest.suite,
    PrettyTest.suite,
    PrintTest.suite,
    RangeTest.suite,
    StringTest.suite,
    TargetTest.suite,
    TupleTest.suite,
  ]
  @ NodeTest.suite;
