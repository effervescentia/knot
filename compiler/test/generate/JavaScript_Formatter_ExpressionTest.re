open Kore;
open Generate.JavaScript_AST;

module Formatter = Generate.JavaScript_Formatter;

let _assert_expression = (expected, actual) =>
  Assert.string(
    expected,
    actual |> ~@Fmt.root(Formatter.fmt_expression(Target.Common)),
  );

let suite =
  "Generate.JavaScript_Formatter | Expression"
  >::: [
    "null" >: (() => _assert_expression("null", Null)),
    "boolean - true" >: (() => _assert_expression("true", Boolean(true))),
    "boolean - false" >: (() => _assert_expression("false", Boolean(false))),
    "number - integer" >: (() => _assert_expression("123", Number("123"))),
    "number - float"
    >: (() => _assert_expression("1234.56", Number("1234.56"))),
    "number - IEEE 754 notation"
    >: (
      () =>
        _assert_expression(
          "1.79769313486e+308",
          Number("1.79769313486e+308"),
        )
    ),
    "string - no special characters"
    >: (() => _assert_expression("\"foobar\"", String("foobar"))),
    "string - escaped newline"
    >: (() => _assert_expression("\"foo\\nbar\"", String("foo\nbar"))),
    "string - escaped quotation mark"
    >: (
      () => _assert_expression("\"foo \\\"bar\\\"\"", String("foo \"bar\""))
    ),
    "group" >: (() => _assert_expression("(null)", Group(Null))),
    "nested group"
    >: (() => _assert_expression("(null)", Group(Group(Group(Null))))),
    "identifier"
    >: (() => _assert_expression("foobar", Identifier("foobar"))),
    "property access"
    >: (
      () =>
        _assert_expression(
          "foo.bar.fizz",
          DotAccess(DotAccess(Identifier("foo"), "bar"), "fizz"),
        )
    ),
    "function call"
    >: (
      () =>
        _assert_expression(
          "foo(true, null)",
          FunctionCall(Identifier("foo"), [Boolean(true), Null]),
        )
    ),
    "method call"
    >: (
      () =>
        _assert_expression(
          "foo.bar()",
          FunctionCall(DotAccess(Identifier("foo"), "bar"), []),
        )
    ),
    "negative number"
    >: (() => _assert_expression("-123", UnaryOp("-", Number("123")))),
    "negated boolean"
    >: (() => _assert_expression("!foo", UnaryOp("!", Identifier("foo")))),
    "addition operation"
    >: (
      () =>
        _assert_expression(
          "123 + 456",
          BinaryOp("+", Number("123"), Number("456")),
        )
    ),
    "ternary operation"
    >: (
      () =>
        _assert_expression(
          "foo ? 123 : 456",
          Ternary(Identifier("foo"), Number("123"), Number("456")),
        )
    ),
    "function - empty"
    >: (() => _assert_expression("function () { }", Function(None, [], []))),
    "function - multiple arguments and expressions in body"
    >: (
      () =>
        _assert_expression(
          "function foo(bar, fizz) {
  var x = 123;
  x + 20;
  return null;
}",
          Function(
            Some("foo"),
            ["bar", "fizz"],
            [
              Variable("x", Number("123")),
              Expression(BinaryOp("+", Identifier("x"), Number("20"))),
              Return(Some(Null)),
            ],
          ),
        )
    ),
    "object - empty" >: (() => _assert_expression("{}", Object([]))),
    "object - multiple properties"
    >: (
      () =>
        _assert_expression(
          "{
  foo: 123,
  bar: 20 - 5
}",
          Object([
            ("foo", Number("123")),
            ("bar", BinaryOp("-", Number("20"), Number("5"))),
          ]),
        )
    ),
  ];
