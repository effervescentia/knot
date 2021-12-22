let tests =
  [
    ASTTest.suite,
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
    RangeTest.suite,
    SeqTest.suite,
    StringTest.suite,
    TargetTest.suite,
    TupleTest.suite,
  ]
  @ ReferenceTest.suite
  @ NodeTest.suite
  @ TypeTest.suite;
