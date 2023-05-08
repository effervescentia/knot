open Kore;
open Generate.JavaScript_AST;

module Formatter = Generate.JavaScript_Formatter;

let _assert_program = (target, expected, actual) =>
  Assert.string(expected, actual |> ~@Fmt.root(Formatter.format(target)));
let _assert_commonjs_program = _assert_program(Target.Common);
let _assert_es6_program = _assert_program(Target.ES6);

let suite =
  "Generate.JavaScript_Formatter | Program"
  >::: [
    "es6"
    >: (
      () =>
        _assert_es6_program(
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
        )
    ),
    "commonjs"
    >: (
      () =>
        _assert_commonjs_program(
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
        )
    ),
  ];
