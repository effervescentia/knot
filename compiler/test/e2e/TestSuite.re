open Test.Infix;

let () = "E2E Tests" >:::: [EveryErrorTest.suite, EveryFeatureTest.suite];
