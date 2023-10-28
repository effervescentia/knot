mod common;

use js::{JavaScriptGenerator, Module};
use std::collections::HashMap;

#[test]
#[ignore = "skip for now"]
fn with_absolute_imports() {
    const INPUT_A: &str = "const VALUE_A = 123;";
    const INPUT_B: &str = "const VALUE_B = 456;";
    const INPUT_ROOT: &str = "use @/a;
use @/deep/b;

const ROOT = 789;
";

    const OUTPUT_A: &str = "import { $knot } from \"@knot/runtime\";
var VALUE_A = 123;
export { VALUE_A };
";
    const OUTPUT_B: &str = "import { $knot } from \"@knot/runtime\";
var VALUE_B = 456;
export { VALUE_B };
";
    const OUTPUT_ROOT: &str = "import { $knot } from \"@knot/runtime\";
var ROOT = 789;
export { ROOT };
";

    let name = common::test_name(file!(), "absolute_imports");
    let result = common::build(
        &name,
        vec![
            ("main.kn", INPUT_ROOT),
            ("a.kn", INPUT_A),
            ("deep/b.kn", INPUT_B),
        ],
        JavaScriptGenerator::new(Module::ESM),
    );

    assert_eq!(
        result.unwrap(),
        HashMap::from_iter(vec![
            (String::from("a.js"), OUTPUT_A.to_string()),
            (String::from("deep/b.js"), OUTPUT_B.to_string()),
            (String::from("main.js"), OUTPUT_ROOT.to_string())
        ])
    );
}

#[test]
#[ignore = "skip for now"]
fn with_relative_imports() {
    const INPUT_A: &str = "const VALUE_A = 123;";
    const INPUT_B: &str = "use ./deeper/a;
const VALUE_B = 456;
";
    const INPUT_ROOT: &str = "use ./deep/b;
const ROOT = 789;
";

    const OUTPUT_A: &str = "import { $knot } from \"@knot/runtime\";
var VALUE_A = 123;
export { VALUE_A };
";
    const OUTPUT_B: &str = "import { $knot } from \"@knot/runtime\";
var VALUE_B = 456;
export { VALUE_B };
";
    const OUTPUT_ROOT: &str = "import { $knot } from \"@knot/runtime\";
var ROOT = 789;
export { ROOT };
";

    let name = common::test_name(file!(), "relative_imports");
    let result = common::build(
        &name,
        vec![
            ("main.kn", INPUT_ROOT),
            ("deep/b.kn", INPUT_B),
            ("deep/deeper/a.kn", INPUT_A),
        ],
        JavaScriptGenerator::new(Module::ESM),
    );

    assert_eq!(
        result.unwrap(),
        HashMap::from_iter(vec![
            (String::from("main.js"), OUTPUT_ROOT.to_string()),
            (String::from("deep/b.js"), OUTPUT_B.to_string()),
            (String::from("deep/deeper/a.js"), OUTPUT_A.to_string()),
        ])
    );
}
