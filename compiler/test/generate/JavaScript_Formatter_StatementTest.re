open Kore;
open Util.ResultUtil;
open Generate.JavaScript_AST;

module Formatter = Generate.JavaScript_Formatter;

let _assert_statement = (target, expected, actual) =>
  Assert.string(
    expected,
    actual |> ~@Fmt.root(Formatter.fmt_statement(target)),
  );
let _assert_commonjs_statement = _assert_statement(Target.Common);
let _assert_es6_statement = _assert_statement(Target.ES6);

let suite =
  "Generate.JavaScript_Formatter | Statement"
  >::: [
    "expression - primitive"
    >: (() => _assert_commonjs_statement("null", Expression(Null))),
    "expression - binary operation"
    >: (
      () =>
        _assert_commonjs_statement(
          "5 < 10",
          Expression(BinaryOp("<", Number("5"), Number("10"))),
        )
    ),
    "variable declaration - primitive"
    >: (
      () =>
        _assert_commonjs_statement("var foo = null", Variable("foo", Null))
    ),
    "variable declaration - expression"
    >: (
      () =>
        _assert_commonjs_statement(
          "var foo = 5 < 10",
          Variable("foo", BinaryOp("<", Number("5"), Number("10"))),
        )
    ),
    "variable assignment"
    >: (
      () =>
        _assert_commonjs_statement(
          "foo = null",
          Assignment(Identifier("foo"), Null),
        )
    ),
    "property assignment"
    >: (
      () =>
        _assert_commonjs_statement(
          "foo.bar = 5 < 10",
          Assignment(
            DotAccess(Identifier("foo"), "bar"),
            BinaryOp("<", Number("5"), Number("10")),
          ),
        )
    ),
    "return - nothing"
    >: (() => _assert_commonjs_statement("return", Return(None))),
    "return - value"
    >: (() => _assert_commonjs_statement("return null", Return(Some(Null)))),
    "es6 import"
    >: (
      () =>
        _assert_es6_statement(
          "import bar from \"foo\"",
          DefaultImport("foo", "bar"),
        )
    ),
    "es6 import - scoped package"
    >: (
      () =>
        _assert_es6_statement(
          "import bar from \"foo/bar\"",
          DefaultImport("foo/bar", "bar"),
        )
    ),
    "es6 import - skip empty imports"
    >: (() => _assert_es6_statement("", Import("foo/bar", []))),
    "es6 import - named import"
    >: (
      () =>
        _assert_es6_statement(
          "import { fizz } from \"foo/bar\"",
          Import("foo/bar", [("fizz", None)]),
        )
    ),
    "es6 import - aliased import"
    >: (
      () =>
        _assert_es6_statement(
          "import { fizz, buzz as local_buzz } from \"foo/bar\"",
          Import(
            "foo/bar",
            [("fizz", None), ("buzz", Some("local_buzz"))],
          ),
        )
    ),
    "es6 export - named export"
    >: (() => _assert_es6_statement("export { foo }", Export("foo", None))),
    "es6 export - aliased export"
    >: (
      () =>
        _assert_es6_statement(
          "export { foo as bar }",
          Export("foo", Some("bar")),
        )
    ),
    "es6 export - empty exports"
    >: (() => _assert_es6_statement("export {}", EmptyExport)),
    "commonjs import"
    >: (
      () =>
        _assert_commonjs_statement(
          "var bar = require(\"foo\")",
          DefaultImport("foo", "bar"),
        )
    ),
    "commonjs import - scoped package"
    >: (
      () =>
        _assert_commonjs_statement(
          "var bar = require(\"foo/bar\")",
          DefaultImport("foo/bar", "bar"),
        )
    ),
    "commonjs import - skip empty imports"
    >: (() => _assert_commonjs_statement("", Import("foo/bar", []))),
    "commonjs import - named import"
    >: (
      () =>
        _assert_commonjs_statement(
          "var $import$foo$bar = require(\"foo/bar\");
var fizz = $import$foo$bar.fizz;
$import$foo$bar = null",
          Import("foo/bar", [("fizz", None)]),
        )
    ),
    "commonjs import - aliased import"
    >: (
      () =>
        _assert_commonjs_statement(
          "var $import$__$foo$bar = require(\"../foo/bar\");
var fizz = $import$__$foo$bar.fizz;
var local_buzz = $import$__$foo$bar.buzz;
$import$__$foo$bar = null",
          Import(
            "../foo/bar",
            [("fizz", None), ("buzz", Some("local_buzz"))],
          ),
        )
    ),
    "commonjs export - named export"
    >: (
      () =>
        _assert_commonjs_statement("exports.foo = foo", Export("foo", None))
    ),
    "commonjs export - aliased export"
    >: (
      () =>
        _assert_commonjs_statement(
          "exports.bar = foo",
          Export("foo", Some("bar")),
        )
    ),
    "commonjs export - empty exports"
    >: (() => _assert_commonjs_statement("module.exports = {}", EmptyExport)),
  ];
