open Test.Infix;

let () =
  "E2E Tests"
  >:::: [
    "Build" >::: ["Every Error" >:: (_ => EveryErrorTest.test())],
    EveryFeatureTest.suite,
  ];
