open Kore;

let tests =
  [
    FilenameTest.suite,
    ErrorTest.suite,
    FmtTest.suite,
    FunTest.suite,
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
    RangeTest.suite,
    StringTest.suite,
    TargetTest.suite,
    TupleTest.suite,
  ]
  @ ReferenceTest.suite
  @ NodeTest.suite
  @ TypeTest.suite;
