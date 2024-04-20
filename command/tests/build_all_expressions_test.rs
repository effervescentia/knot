#![allow(clippy::indexing_slicing)]
mod common;

use common::assert_build_single;

fn assert_build_expression(name: &str, input: &str, output: &str) {
    let source = format!("const VALUE = {input};");
    let compiled = format!(
        "import {{ $knot }} from \"@knot/runtime\";
var VALUE = {output};
export {{ VALUE }};
",
    );

    assert_build_single(name, &source, &compiled);
}

fn assert_build_expressions(name: &str, pairs: &[(&str, &str)]) {
    let (source, (bindings, exports)) = pairs
        .iter()
        .enumerate()
        .map(|(index, (input, output))| {
            (
                format!("const VALUE_{index} = {input};"),
                (
                    format!("var VALUE_{index} = {output};\n"),
                    format!("export {{ VALUE_{index} }};\n"),
                ),
            )
        })
        .unzip::<_, _, String, (String, String)>();

    let compiled = format!(
        "import {{ $knot }} from \"@knot/runtime\";
{bindings}{exports}",
    );

    assert_build_single(name, &source, &compiled);
}

#[test]
fn nil() {
    assert_build_expression(stdext::function_name!(), "nil", "null");
}

#[test]
fn boolean() {
    assert_build_expressions(
        stdext::function_name!(),
        &[("true", "true"), ("false", "false")],
    );
}

#[test]
fn integer() {
    assert_build_expressions(
        stdext::function_name!(),
        &[("0", "0"), ("123", "123"), ("-456", "-456")],
    );
}

#[test]
fn float() {
    assert_build_expressions(
        stdext::function_name!(),
        &[
            ("0.0", "0.0"),
            ("123.0", "123.0"),
            ("-4.56", "-4.56"),
            ("0.07", "0.07"),
        ],
    );
}

#[test]
fn string() {
    assert_build_expressions(
        stdext::function_name!(),
        &[("\"\"", "\"\""), ("\"hello, world!\"", "\"hello, world!\"")],
    );
}

#[test]
fn identifier() {
    const INPUT: &str = "
const ORIGINAL = 10;
const REFERENCE = ORIGINAL;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
var ORIGINAL = 10;
var REFERENCE = ORIGINAL;
export { ORIGINAL };
export { REFERENCE };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn group() {
    assert_build_expressions(
        stdext::function_name!(),
        &[("(nil)", "(null)"), ("(((nil)))", "(((null)))")],
    );
}

#[test]
fn closure() {
    assert_build_expressions(
        stdext::function_name!(),
        &[
            ("{}", "null"),
            (
                "{
  let left = 3;
  let right = 10;

  left + right;
}",
                "(function() {
  var left = 3;
  var right = 10;
  return left + right;
})()",
            ),
        ],
    );
}

#[test]
fn unary_operation() {
    assert_build_expressions(
        stdext::function_name!(),
        &[
            ("!true", "!true"),
            // TODO: this should work but it doesn't
            // ("!!false", "!!false"),
            ("+123", "Math.abs(123)"),
            ("-123", "-123"),
        ],
    );
}

#[test]
fn binary_operation() {
    assert_build_expressions(
        stdext::function_name!(),
        &[
            ("true && true", "true && true"),
            ("false || true", "false || true"),
            ("8 == 8", "8 === 8"),
            ("3 != 5", "3 !== 5"),
            ("1 < 10", "1 < 10"),
            ("4 <= 4", "4 <= 4"),
            ("10 > 1", "10 > 1"),
            ("4 >= 4", "4 >= 4"),
            ("2 + 2", "2 + 2"),
            ("8 - 4", "8 - 4"),
            ("2 * 2", "2 * 2"),
            ("8 / 2", "8 / 2"),
            ("2 ^ 2", "Math.pow(2, 2)"),
        ],
    );
}

#[test]
fn property_access() {
    const INPUT: &str = "
module container {
  const VALUE = 123;
}

const PROPERTY = container.VALUE;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
var container = (function() {
  var VALUE = 123;
  return {
    VALUE: VALUE,
  };
})();
var PROPERTY = container.VALUE;
export { container };
export { PROPERTY };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn function_call() {
    const INPUT: &str = "
func add(left: integer, right: integer) -> left + right;

const SUM = add(2, 4);
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
function add(left, right) {
  return left + right;
}
var SUM = add(2, 4);
export { add };
export { SUM };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}

#[test]
fn style() {
    assert_build_expressions(
        stdext::function_name!(),
        &[
            (
                "style {}",
                "$knot.plugin.get(\"style\", \"create\", \"1.0\")({})",
            ),
            // TODO: add tests for styles with css properties
        ],
    );
}

#[test]
// TODO: add tests for components with html elements
fn component() {
    const INPUT: &str = "
view Static -> \"Welcome!\";

const VALUE = <Static />;
";

    const OUTPUT: &str = "import { $knot } from \"@knot/runtime\";
function Static() {
  return \"Welcome!\";
}
var VALUE = $knot.plugin.get(\"ksx\", \"createElement\", \"1.0\")(Static);
export { Static };
export { VALUE };
";

    assert_build_single(stdext::function_name!(), INPUT, OUTPUT);
}
