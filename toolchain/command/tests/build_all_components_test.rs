#![allow(clippy::indexing_slicing)]
mod common;

use common::assert_build_single;

fn assert_build_component(name: &str, input: &str, output: &str) {
    let source = format!("view View -> <>{input}</>;");
    let compiled = format!(
        "import {{ $knot }} from \"@knot/runtime\";
function View() {{
  return $knot.plugin.get(\"view\", \"createFragment\", \"1.0\")({output});
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
fn fragment() {
    assert_build_component(
        stdext::function_name!(),
        "<></>",
        "$knot.plugin.get(\"view\", \"createFragment\", \"1.0\")()",
    );
}

#[test]
fn closed_element() {
    assert_build_component(
        stdext::function_name!(),
        "<div id=\"root\" />",
        "$knot.plugin.get(\"view\", \"createElement\", \"1.0\")(\"div\", {
    id: \"root\",
  })",
    );
}

#[test]
fn open_element() {
    assert_build_component(
        stdext::function_name!(),
        "<div id=\"root\">hello</div>",
        "$knot.plugin.get(\"view\", \"createElement\", \"1.0\")(\"div\", {
    id: \"root\",
  }, \"hello\")",
    );
}
