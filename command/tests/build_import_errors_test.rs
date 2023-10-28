mod common;

use js::{JavaScriptGenerator, Module};
use knot_command::{engine, Link};

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

    assert_eq!(
        result,
        Err(vec![engine::Error::ImportCycle(vec![
            Link::from("b"),
            Link::from("a"),
            Link::from("c"),
        ])])
    )
}
