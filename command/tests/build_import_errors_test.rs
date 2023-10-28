mod common;

use js::{JavaScriptGenerator, Module};
use std::collections::HashMap;

#[test]
fn cyclic() {
    const INPUT_A: &str = "use @/b;";
    const INPUT_B: &str = "use @/c;";
    const INPUT_C: &str = "use @/a;";

    let name = common::test_name(file!(), "absolute_imports");
    let result = common::build(
        &name,
        vec![("a.kn", INPUT_A), ("b.kn", INPUT_B), ("c.kn", INPUT_C)],
        JavaScriptGenerator::new(Module::ESM),
    );

    // assert_eq!(
    //     result,
    //     HashMap::from_iter(vec![
    //         (String::from("a.js"), OUTPUT_A.to_string()),
    //         (String::from("deep/b.js"), OUTPUT_B.to_string()),
    //         (String::from("main.js"), INP.to_string())
    //     ])
    // );
}
