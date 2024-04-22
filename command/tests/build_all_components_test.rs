#![allow(clippy::indexing_slicing)]
mod common;

use common::assert_build_single;

fn assert_build_component(name: &str, input: &str, output: &str) {
    let source = format!("view View -> <>{input}</>;");
    let compiled = format!(
        "import {{ $knot }} from \"@knot/runtime\";
function View() {{
  return $knot.plugin.get(\"ksx\", \"createFragment\", \"1.0\")({output});
}}
export {{ View }};
",
    );

    assert_build_single(name, &source, &compiled);
}

#[test]
fn text() {
    assert_build_component(stdext::function_name!(), "hello, world", "\"hello, world\"");
}

#[test]
fn expression() {
    assert_build_component(stdext::function_name!(), "{2 + 3}", "2 + 3");
}

#[test]
fn closed_element() {
    assert_build_component(
        stdext::function_name!(),
        "<div id=\"root\" />",
        "$knot.plugin.get(\"ksx\", \"createElement\", \"1.0\")(\"div\", {
    id: \"root\",
  })",
    );
}

// TODO: add better tests for fragments & elements when HTML is in scope
