// validating the parse -> format -> parse cycle

use knot_language::{parser, Program};

fn parse(s: &str) -> parser::Result<Program<()>> {
    parser::parse(s)
}

#[test]
fn empty_module() {
    let source = "";

    let ast = parse(source).unwrap().0;

    assert_eq!(ast.to_string(), source);
}

#[test]
fn multiple_declarations() {
    let source = "type MyTypeAlias = nil;
enum MyEnum =
  | First
  | Second(boolean, style);
const MY_CONST: string = \"hello, world!\";
func my_func(first, second: integer, third = true): boolean -> first > second || third;
view MyView(only: element = <div />) -> {
  let value = 123 + 45.67;
  <>
    <h1>Welcome!</h1>
    <main>{value}: {only}</main>
  </>;
};
module inner {
  const MY_STYLE = style {
    color: \"red\",
    display: \"block\",
  };
}
";

    let ast = parse(source).unwrap().0;

    assert_eq!(ast.to_string(), source);
}

#[test]
fn mixed_ksx() {
    let source = "const MIXED = <div>
  hello {name}, how are you doing?
  <div />
  {left} or {right}
  <div />
  that's all folks!
</div>;
";

    let ast = parse(source).unwrap().0;

    assert_eq!(ast.to_string(), source);
}
