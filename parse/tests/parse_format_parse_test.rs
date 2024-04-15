// validating the parse -> format -> parse cycle
use kore::assert_str_eq;
use lang::ast;

fn parse_program(s: &str) -> knot_parse::Result<ast::raw::Program> {
    knot_parse::program::parse(s)
}

fn parse_typings(s: &str) -> knot_parse::Result<ast::raw::Typings> {
    knot_parse::typings::parse(s)
}

#[test]
fn empty_module() {
    let source = "";

    let ast = parse_program(source).unwrap().0;

    assert_str_eq!(ast.to_shape().to_string(), source);
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

    let ast = parse_program(source).unwrap().0;

    assert_str_eq!(ast.to_shape().to_string(), source);
}

#[test]
fn mixed_components() {
    let source = "const MIXED = <div>
  hello {name}, how are you doing?
  <div />
  {left} or {right}
  <div />
  that's all folks!
</div>;
";

    let ast = parse_program(source).unwrap().0;

    assert_str_eq!(ast.to_shape().to_string(), source);
}

#[test]
fn typings() {
    let source = "type Props = {
  foo: integer,
};
view Bar (Props);
";

    let ast = parse_typings(source).unwrap().0;

    assert_str_eq!(ast.to_shape().to_string(), source);
}
