open Alcotest;

let (>::::) = (label, suite) => run(label, suite);
let (>:::) = (label, tests) => (label, tests);
let (>::) = (label, test) => test_case(label, `Slow, test);
let (>:) = (label, test) => test_case(label, `Quick, test);
