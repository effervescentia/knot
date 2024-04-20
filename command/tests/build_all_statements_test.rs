#![allow(clippy::indexing_slicing)]
mod common;

use common::assert_build_single;

fn assert_build_statement(name: &str, input: &str, output: &str) {
    let source = format!(
        "
const CONTAINER = {{
  {input}
}};
"
    );
    let compiled = format!(
        "import {{ $knot }} from \"@knot/runtime\";
var CONTAINER = (function() {{
  {output}
}})();
export {{ CONTAINER }};
",
    );

    assert_build_single(name, &source, &compiled);
}

#[test]
fn expression() {
    assert_build_statement(stdext::function_name!(), "2 + 7;", "return 2 + 7;");
}

#[test]
fn variable() {
    assert_build_statement(
        stdext::function_name!(),
        "let foo = 12;",
        "var foo = 12;\n  return null;",
    );
}
