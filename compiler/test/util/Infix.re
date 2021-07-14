let (>::::) = (label, suite) => Alcotest.run(label, suite);
let (>:::) = (label, tests) => (label, tests);
let (>::) = (label, test) => Alcotest.test_case(label, `Slow, test);
let (>:) = (label, test) => Alcotest.test_case(label, `Quick, test);

let (>~::::) = (label, suite) => Lwt_main.run @@ Async.run(label, suite);
let (>~::) = (label, test) => Async.test_case(label, `Slow, test);
let (>~:) = (label, test) => Async.test_case(label, `Quick, test);
