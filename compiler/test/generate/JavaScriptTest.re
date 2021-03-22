open Kore;
open Util;

module JavaScript = Generate.JavaScript;

let _in_block = x => Block.create(Cursor.zero, x);

let __resolved = "../foo/bar";
let __program = [
  ("foo/bar" |> of_internal, "Foo") |> of_import,
  ("ABC" |> of_public |> as_lexeme, 123 |> int_prim |> of_const) |> of_decl,
];

let suite =
  "Generate.JavaScript"
  >::: [
    "number()"
    >: (
      () => {
        [
          ("123", 123 |> Int64.of_int |> of_int |> JavaScript.number),
          (
            "9223372036854775807",
            Int64.max_int |> of_int |> JavaScript.number,
          ),
          (
            "-9223372036854775808",
            Int64.min_int |> of_int |> JavaScript.number,
          ),
          ("0", (0.0, 0) |> of_float |> JavaScript.number),
          ("1", (1.0, 0) |> of_float |> JavaScript.number),
          ("12345", (12345.0, 5) |> of_float |> JavaScript.number),
          ("4.56", (4.56, 3) |> of_float |> JavaScript.number),
          ("1234.56", (1234.56, 6) |> of_float |> JavaScript.number),
          ("0.123", (0.123, 3) |> of_float |> JavaScript.number),
          ("0.000456", (0.000456, 6) |> of_float |> JavaScript.number),
          (
            "1.79769313486e+308",
            (Float.max_float, 12) |> of_float |> JavaScript.number,
          ),
          (
            "2.22507385851e-308",
            (Float.min_float, 12) |> of_float |> JavaScript.number,
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "primitive()"
    >: (
      () => {
        let print = x => {
          let buffer = Buffer.create(1);

          JavaScript.primitive(Buffer.add_string(buffer), x);

          Buffer.contents(buffer);
        };

        [
          ("true", true |> of_bool |> print),
          ("false", false |> of_bool |> print),
          ("\"hello world\"", "hello world" |> of_string |> print),
          (
            "\"escaped quotes (\\\")\"",
            "escaped quotes (\")" |> of_string |> print,
          ),
          ("null", nil |> print),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "expression()"
    >: (
      () => {
        let print = x => {
          let buffer = Buffer.create(1);

          JavaScript.expression(Buffer.add_string(buffer), x);

          Buffer.contents(buffer);
        };

        [
          ("fooBar", "fooBar" |> of_public |> as_lexeme |> of_id |> print),
          ("(123)", 123 |> int_prim |> of_group |> print),
          (
            "(function(){
(123 === 456);
return (678 + 910);
})()",
            [
              (123 |> int_prim, 456 |> int_prim)
              |> of_eq_op
              |> as_int
              |> of_expr,
              (678 |> int_prim, 910 |> int_prim)
              |> of_add_op
              |> as_int
              |> of_expr,
            ]
            |> of_closure
            |> print,
          ),
          (
            "(function(){
var foo = 456;
return null;
})()",
            [("foo" |> of_public |> as_lexeme, 456 |> int_prim) |> of_var]
            |> of_closure
            |> print,
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "statement()"
    >: (
      () => {
        let print = x => {
          let buffer = Buffer.create(1);

          JavaScript.statement(Buffer.add_string(buffer), x);

          Buffer.contents(buffer);
        };

        [
          (
            "var fooBar = 123;\n",
            ("fooBar" |> of_public |> as_lexeme, 123 |> int_prim)
            |> of_var
            |> print,
          ),
          (
            "(123 === 456);\n",
            (123 |> int_prim, 456 |> int_prim)
            |> of_eq_op
            |> as_int
            |> of_expr
            |> print,
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "binary_op()"
    >: (
      () => {
        let print = (x, y, z) => {
          let buffer = Buffer.create(1);

          JavaScript.binary_op(Buffer.add_string(buffer), x, y, z);

          Buffer.contents(buffer);
        };

        [
          (
            "(true && false)",
            print(LogicalAnd, true |> bool_prim, false |> bool_prim),
          ),
          (
            "(true || false)",
            print(LogicalOr, true |> bool_prim, false |> bool_prim),
          ),
          (
            "(123 <= 456)",
            print(LessOrEqual, 123 |> int_prim, 456 |> int_prim),
          ),
          ("(123 < 456)", print(LessThan, 123 |> int_prim, 456 |> int_prim)),
          (
            "(123 >= 456)",
            print(GreaterOrEqual, 123 |> int_prim, 456 |> int_prim),
          ),
          (
            "(123 > 456)",
            print(GreaterThan, 123 |> int_prim, 456 |> int_prim),
          ),
          ("(123 === 456)", print(Equal, 123 |> int_prim, 456 |> int_prim)),
          (
            "(123 !== 456)",
            print(Unequal, 123 |> int_prim, 456 |> int_prim),
          ),
          ("(123 + 456)", print(Add, 123 |> int_prim, 456 |> int_prim)),
          ("(123 - 456)", print(Subtract, 123 |> int_prim, 456 |> int_prim)),
          ("(123 * 456)", print(Multiply, 123 |> int_prim, 456 |> int_prim)),
          ("(123 / 456)", print(Divide, 123 |> int_prim, 456 |> int_prim)),
          (
            "Math.pow(123, 456)",
            print(Exponent, 123 |> int_prim, 456 |> int_prim),
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "unary_op()"
    >: (
      () => {
        let print = (x, y) => {
          let buffer = Buffer.create(1);

          JavaScript.unary_op(Buffer.add_string(buffer), x, y);

          Buffer.contents(buffer);
        };

        [
          ("!true", print(Not, true |> bool_prim)),
          ("+123", print(Positive, 123 |> int_prim)),
          ("-123", print(Negative, 123 |> int_prim)),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "jsx()"
    >: (
      () => {
        let print = x => {
          let buffer = Buffer.create(1);

          JavaScript.jsx(Buffer.add_string(buffer), x);

          Buffer.contents(buffer);
        };

        [
          (
            "$knot.jsx.createTag(\"Foo\", {})",
            ("Foo" |> of_public |> as_lexeme, [], []) |> of_tag |> print,
          ),
          ("$knot.jsx.createFragment()", [] |> of_frag |> print),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "jsx_child()"
    >: (
      () => {
        let print = x => {
          let buffer = Buffer.create(1);

          JavaScript.jsx_child(Buffer.add_string(buffer), x);

          Buffer.contents(buffer);
        };

        [
          (
            "$knot.jsx.createTag(\"Foo\", {})",
            ("Foo" |> of_public |> as_lexeme, [], [])
            |> of_tag
            |> as_lexeme
            |> of_node
            |> print,
          ),
          (
            "$knot.jsx.createTag(\"Foo\", { bar: fizz, foo: foo })",
            (
              "Foo" |> of_public |> as_lexeme,
              [
                ("foo" |> of_public |> as_lexeme, None) |> of_prop |> as_lexeme,
                (
                  "bar" |> of_public |> as_lexeme,
                  Some(
                    "fizz" |> of_public |> as_lexeme |> of_id |> as_unknown,
                  ),
                )
                |> of_prop
                |> as_lexeme,
              ],
              [],
            )
            |> of_tag
            |> as_lexeme
            |> of_node
            |> print,
          ),
          (
            "$knot.jsx.createTag(\"Foo\", {}, $knot.jsx.createTag(\"Bar\", {}, \"fizz\"))",
            (
              "Foo" |> of_public |> as_lexeme,
              [],
              [
                (
                  "Bar" |> of_public |> as_lexeme,
                  [],
                  ["fizz" |> of_text |> as_lexeme],
                )
                |> of_tag
                |> as_lexeme
                |> of_node
                |> as_lexeme,
              ],
            )
            |> of_tag
            |> as_lexeme
            |> of_node
            |> print,
          ),
          ("\"Hello World!\"", "Hello World!" |> of_text |> print),
          ("123", 123 |> int_prim |> of_inline_expr |> print),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "jsx_attrs()"
    >: (
      () => {
        let print = x => {
          let buffer = Buffer.create(1);

          JavaScript.jsx_attrs(Buffer.add_string(buffer), x);

          Buffer.contents(buffer);
        };

        [
          (
            "{ foo: foo }",
            [("foo" |> of_public |> as_lexeme, None) |> of_prop |> as_lexeme]
            |> print,
          ),
          (
            "{ foo: bar }",
            [
              (
                "foo" |> of_public |> as_lexeme,
                Some("bar" |> of_public |> as_lexeme |> of_id |> as_unknown),
              )
              |> of_prop
              |> as_lexeme,
            ]
            |> print,
          ),
          (
            "{ className: \".foo\" }",
            [
              ("foo" |> of_public |> as_lexeme, None)
              |> of_jsx_class
              |> as_lexeme,
            ]
            |> print,
          ),
          (
            "{ className: ((123 > 456) ? \".foo\" : \"\") + \".bar\" }",
            [
              ("bar" |> of_public |> as_lexeme, None)
              |> of_jsx_class
              |> as_lexeme,
              (
                "foo" |> of_public |> as_lexeme,
                Some(
                  (123 |> int_prim, 456 |> int_prim) |> of_gt_op |> as_bool,
                ),
              )
              |> of_jsx_class
              |> as_lexeme,
            ]
            |> print,
          ),
          (
            "{ id: \"foo\" }",
            ["foo" |> of_public |> as_lexeme |> of_jsx_id |> as_lexeme]
            |> print,
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "constant()"
    >: (
      () => {
        let print = (x, y) => {
          let buffer = Buffer.create(1);

          JavaScript.constant(Buffer.add_string(buffer), x, y);

          Buffer.contents(buffer);
        };

        [
          (
            "var foo = 123;\n",
            123 |> int_prim |> print("foo" |> of_public |> as_lexeme),
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "declaration()"
    >: (
      () => {
        let print = (module_type, x, y) => {
          let buffer = Buffer.create(1);

          JavaScript.declaration(
            Buffer.add_string(buffer),
            module_type,
            x,
            y,
          );

          Buffer.contents(buffer);
        };

        [
          (
            "var foo = 123;
exports.foo = foo;
",
            123
            |> int_prim
            |> of_const
            |> print(Target.Common, "foo" |> of_public |> as_lexeme),
          ),
          (
            "var foo = 123;
export { foo };
",
            123
            |> int_prim
            |> of_const
            |> print(Target.ES6, "foo" |> of_public |> as_lexeme),
          ),
        ]
        |> Assert.(test_many(string));
      }
    ),
    "generate() - empty commonJS module"
    >: (
      () => {
        let buffer = Buffer.create(100);
        let print = Buffer.add_string(buffer);

        JavaScript.generate(Common, {print, resolve: _ => ""}, []);

        Assert.string("module.exports = {};\n", buffer |> Buffer.contents);
      }
    ),
    "generate() - empty ES6 module"
    >: (
      () => {
        let buffer = Buffer.create(100);
        let print = Buffer.add_string(buffer);

        JavaScript.generate(ES6, {print, resolve: _ => ""}, []);

        Assert.string("export {};\n", buffer |> Buffer.contents);
      }
    ),
    "generate() - with commonJS imports and exports"
    >: (
      () => {
        let buffer = Buffer.create(100);
        let print = Buffer.add_string(buffer);

        JavaScript.generate(
          Common,
          {
            print,
            resolve: path => {
              Assert.namespace("foo/bar" |> of_internal, path);
              __resolved;
            },
          },
          __program,
        );

        Assert.string(
          "var $knot = require(\"@knot/runtime\");
var Foo = require(\"../foo/bar\");
var ABC = 123;
exports.ABC = ABC;
",
          buffer |> Buffer.contents,
        );
      }
    ),
    "generate() - with ES6 imports and exports"
    >: (
      () => {
        let buffer = Buffer.create(100);
        let print = Buffer.add_string(buffer);

        JavaScript.generate(
          ES6,
          {
            print,
            resolve: path => {
              Assert.namespace("foo/bar" |> of_internal, path);
              __resolved;
            },
          },
          __program,
        );

        Assert.string(
          "import $knot from \"@knot/runtime\";
import Foo from \"../foo/bar\";
var ABC = 123;
export { ABC };
",
          buffer |> Buffer.contents,
        );
      }
    ),
  ];
