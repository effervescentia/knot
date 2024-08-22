#![allow(clippy::indexing_slicing)]
mod common;

use common::assert_build_single;
use std::fmt::Write;

fn assert_build_type_expressions(name: &str, inputs: &[&str]) {
    let source = inputs
        .iter()
        .enumerate()
        .fold(String::new(), |mut acc, (index, input)| {
            write!(&mut acc, "type TYPE_{index} = {input};").ok();
            acc
        });

    let compiled = "import { $knot } from \"@knot/runtime\";\n";

    assert_build_single(name, &source, compiled);
}

#[test]
fn primitive() {
    assert_build_type_expressions(
        stdext::function_name!(),
        &[
            "nil", "boolean", "integer", "float", "string", "style", "element",
        ],
    );
}

#[test]
fn identifier() {
    const INPUT: &str = "
type Original = nil;
type Reference = Original;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";\n";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn group() {
    assert_build_type_expressions(stdext::function_name!(), &["(nil)", "(((nil)))"]);
}

#[test]
#[ignore = "not working, maybe it was never implemented?"]
fn property_access() {
    const INPUT: &str = "
type Container = { value: nil };

type Property = Container.value;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";\n";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn function() {
    assert_build_type_expressions(
        stdext::function_name!(),
        &["() -> nil", "(boolean, integer) -> nil"],
    );
}

#[test]
fn object() {
    assert_build_type_expressions(
        stdext::function_name!(),
        &["{}", "{ foo: boolean, bar: string }"],
    );
}
