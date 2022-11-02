let tests =
  [
    ArgumentTest.suite,
    ASTTest.suite,
    FilenameTest.suite,
    ErrorTest.suite,
    FunTest.suite,
    GraphTest.suite,
    HashtblTest.suite,
    InfixTest.suite,
    InputTest.suite,
    IntTest.suite,
    LazyStreamTest.suite,
    ListTest.suite,
    ModuleTableTest.suite,
    ParseContextTest.suite,
    PointTest.suite,
    RangeTest.suite,
    SeqTest.suite,
    StringTest.suite,
    TargetTest.suite,
    TupleTest.suite,
    TypeTest.suite,
  ]
  @ ReferenceTest.suite
  @ NodeTest.suite;
