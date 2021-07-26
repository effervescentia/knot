open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Formatter = Generate.JavaScript_Formatter;

let __resolved = "../foo/bar";
let __program = [
  (
    "foo/bar" |> of_internal,
    ["Foo" |> of_public |> as_lexeme |> of_main_import |> as_lexeme],
  )
  |> of_import,
  (
    "ABC" |> of_public |> as_lexeme |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )
  |> of_decl,
];

module Compare = {
  open Alcotest;

  let expression = (module_type: Target.module_t) =>
    testable(
      pp =>
        Formatter.fmt_expression(module_type)
        % Pretty.to_string
        % Format.pp_print_string(pp),
      (==),
    );

  let statement = (module_type: Target.module_t) =>
    testable(
      pp =>
        Formatter.fmt_statement(module_type)
        % Pretty.to_string
        % Format.pp_print_string(pp),
      (==),
    );
};

let _assert_expression = (~module_type=Target.Common) =>
  Alcotest.(
    check(Compare.expression(module_type), "javascript expression matches")
  );

let _assert_statement = (~module_type=Target.Common) =>
  Alcotest.(
    check(Compare.statement(module_type), "javascript statement matches")
  );
let _assert_statement_list = (~module_type=Target.Common) =>
  Alcotest.(
    check(
      list(Compare.statement(module_type)),
      "javascript statement matches",
    )
  );

let suite =
  "Generate.JavaScript_Formatter"
  >::: [
    "fmt_string()"
    >: (
      () =>
        [
          ("\"foo\"", "foo"),
          ("\"foo\\nbar\"", "foo\nbar"),
          ("\"foo \\\"bar\\\"\"", "foo \"bar\""),
        ]
        |> List.map(Tuple.map_snd2(Formatter.fmt_string % Pretty.to_string))
        |> Assert.(test_many(string))
    ),
    "fmt_expression()"
    >: (
      () =>
        [
          ("null", Null),
          ("true", Boolean(true)),
          ("false", Boolean(false)),
          ("123", Number("123")),
          ("1234.56", Number("1234.56")),
          ("1.79769313486e+308", Number("1.79769313486e+308")),
          ("\"foobar\"", String("foobar")),
          ("(null)", Group(Null)),
          ("(null)", Group(Group(Group(Null)))),
          ("foobar", Identifier("foobar")),
          (
            "foo.bar.fizz",
            DotAccess(DotAccess(Identifier("foo"), "bar"), "fizz"),
          ),
          (
            "foo(true, null)",
            FunctionCall(Identifier("foo"), [Boolean(true), Null]),
          ),
          (
            "foo.bar()",
            FunctionCall(DotAccess(Identifier("foo"), "bar"), []),
          ),
          ("-123", UnaryOp("-", Number("123"))),
          ("!foo", UnaryOp("!", Identifier("foo"))),
          ("123 + 456", BinaryOp("+", Number("123"), Number("456"))),
          (
            "foo ? 123 : 456",
            Ternary(Identifier("foo"), Number("123"), Number("456")),
          ),
          ("function () {\n}", Function(None, [], [])),
          (
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
          ),
          ("{}", Object([])),
          (
            "{
  foo: 123,
  bar: 20 - 5
}",
            Object([
              ("foo", Number("123")),
              ("bar", BinaryOp("-", Number("20"), Number("5"))),
            ]),
          ),
        ]
        |> List.map(
             Tuple.map_snd2(
               Formatter.fmt_expression(Target.Common) % Pretty.to_string,
             ),
           )
        |> Assert.(test_many(string))
    ),
    "fmt_statement() - not module specific"
    >: (
      () =>
        [
          ("null", Expression(Null)),
          (
            "5 < 10",
            Expression(BinaryOp("<", Number("5"), Number("10"))),
          ),
          ("var foo = null", Variable("foo", Null)),
          (
            "var foo = 5 < 10",
            Variable("foo", BinaryOp("<", Number("5"), Number("10"))),
          ),
          ("foo = null", Assignment(Identifier("foo"), Null)),
          (
            "foo.bar = 5 < 10",
            Assignment(
              DotAccess(Identifier("foo"), "bar"),
              BinaryOp("<", Number("5"), Number("10")),
            ),
          ),
          ("return", Return(None)),
          ("return null", Return(Some(Null))),
        ]
        |> List.map(
             Tuple.map_snd2(
               Formatter.fmt_statement(Target.Common) % Pretty.to_string,
             ),
           )
        |> Assert.(test_many(string))
    ),
    "fmt_statement() - ES6"
    >: (
      () =>
        [
          ("import bar from \"foo\"", DefaultImport("foo", "bar")),
          ("import bar from \"foo/bar\"", DefaultImport("foo/bar", "bar")),
          ("", Import("foo/bar", [])),
          (
            "import { fizz } from \"foo/bar\"",
            Import("foo/bar", [("fizz", None)]),
          ),
          (
            "import { fizz, buzz as local_buzz } from \"foo/bar\"",
            Import(
              "foo/bar",
              [("fizz", None), ("buzz", Some("local_buzz"))],
            ),
          ),
          ("export { foo }", Export("foo", None)),
          ("export { foo as bar }", Export("foo", Some("bar"))),
          ("export {}", EmptyExport),
        ]
        |> List.map(
             Tuple.map_snd2(
               Formatter.fmt_statement(Target.ES6) % Pretty.to_string,
             ),
           )
        |> Assert.(test_many(string))
    ),
    "fmt_statement() - CommonJS"
    >: (
      () =>
        [
          ("var bar = require(\"foo\")", DefaultImport("foo", "bar")),
          (
            "var bar = require(\"foo/bar\")",
            DefaultImport("foo/bar", "bar"),
          ),
          ("", Import("foo/bar", [])),
          (
            "var $import$foo$bar = require(\"foo/bar\");
var fizz = $import$foo$bar.fizz;
$import$foo$bar = null",
            Import("foo/bar", [("fizz", None)]),
          ),
          (
            "var $import$__$foo$bar = require(\"../foo/bar\");
var fizz = $import$__$foo$bar.fizz;
var local_buzz = $import$__$foo$bar.buzz;
$import$__$foo$bar = null",
            Import(
              "../foo/bar",
              [("fizz", None), ("buzz", Some("local_buzz"))],
            ),
          ),
          ("exports.foo = foo", Export("foo", None)),
          ("exports.bar = foo", Export("foo", Some("bar"))),
          ("module.exports = {}", EmptyExport),
        ]
        |> List.map(
             Tuple.map_snd2(
               Formatter.fmt_statement(Target.Common) % Pretty.to_string,
             ),
           )
        |> Assert.(test_many(string))
    ),
    "format() - ES6"
    >: (
      () =>
        [
          (
            "import bar from \"foo\";
import { fizz } from \"foo/bar\";
var foo = 5 < 10;
export { foo };
export { fizz as buzz };
foo.bar = null;
",
            [
              DefaultImport("foo", "bar"),
              Import("foo/bar", [("fizz", None)]),
              Variable("foo", BinaryOp("<", Number("5"), Number("10"))),
              Export("foo", None),
              Export("fizz", Some("buzz")),
              Assignment(DotAccess(Identifier("foo"), "bar"), Null),
            ],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(Formatter.format(Target.ES6) % Pretty.to_string),
           )
        |> Assert.(test_many(string))
    ),
    "format() - CommonJS"
    >: (
      () =>
        [
          (
            "var bar = require(\"foo\");
var $import$foo$bar = require(\"foo/bar\");
var fizz = $import$foo$bar.fizz;
$import$foo$bar = null;
var foo = 5 < 10;
exports.foo = foo;
exports.buzz = fizz;
foo.bar = null;
",
            [
              DefaultImport("foo", "bar"),
              Import("foo/bar", [("fizz", None)]),
              Variable("foo", BinaryOp("<", Number("5"), Number("10"))),
              Export("foo", None),
              Export("fizz", Some("buzz")),
              Assignment(DotAccess(Identifier("foo"), "bar"), Null),
            ],
          ),
        ]
        |> List.map(
             Tuple.map_snd2(
               Formatter.format(Target.Common) % Pretty.to_string,
             ),
           )
        |> Assert.(test_many(string))
    ),
  ];
