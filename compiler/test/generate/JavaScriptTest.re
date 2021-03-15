open Kore;

module JavaScript = Generate.JavaScript;

let _in_block = x => Block.create(Cursor.zero, x);

let _bool_prim = of_bool % _in_block % of_prim;
let _int_prim = Int64.of_int % of_int % of_num % _in_block % of_prim;

let __resolved = "../foo/bar";
let __program = [
  ("foo/bar" |> of_internal, "Foo") |> of_import,
  ("ABC", 123 |> _int_prim |> of_const) |> of_decl,
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
          ("fooBar", "fooBar" |> _in_block |> of_id |> print),
          ("(123)", 123 |> _int_prim |> _in_block |> of_group |> print),
          (
            "(function(){
(123 === 456);
return (678 + 910);
})()",
            [
              (123 |> _int_prim, 456 |> _int_prim) |> of_eq_op |> of_expr,
              (678 |> _int_prim, 910 |> _int_prim) |> of_add_op |> of_expr,
            ]
            |> _in_block
            |> of_closure
            |> print,
          ),
          (
            "(function(){
var foo = 456;
return null;
})()",
            [("foo", 456 |> _int_prim) |> of_var]
            |> _in_block
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
            ("fooBar", 123 |> _int_prim) |> of_var |> print,
          ),
          (
            "(123 === 456);\n",
            (123 |> _int_prim, 456 |> _int_prim)
            |> of_eq_op
            |> of_expr
            |> print,
          ),
          ("", EmptyStatement |> print),
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
            print(LogicalAnd, true |> _bool_prim, false |> _bool_prim),
          ),
          (
            "(true || false)",
            print(LogicalOr, true |> _bool_prim, false |> _bool_prim),
          ),
          (
            "(123 <= 456)",
            print(LessOrEqual, 123 |> _int_prim, 456 |> _int_prim),
          ),
          (
            "(123 < 456)",
            print(LessThan, 123 |> _int_prim, 456 |> _int_prim),
          ),
          (
            "(123 >= 456)",
            print(GreaterOrEqual, 123 |> _int_prim, 456 |> _int_prim),
          ),
          (
            "(123 > 456)",
            print(GreaterThan, 123 |> _int_prim, 456 |> _int_prim),
          ),
          (
            "(123 === 456)",
            print(Equal, 123 |> _int_prim, 456 |> _int_prim),
          ),
          (
            "(123 !== 456)",
            print(Unequal, 123 |> _int_prim, 456 |> _int_prim),
          ),
          ("(123 + 456)", print(Add, 123 |> _int_prim, 456 |> _int_prim)),
          (
            "(123 - 456)",
            print(Subtract, 123 |> _int_prim, 456 |> _int_prim),
          ),
          (
            "(123 * 456)",
            print(Multiply, 123 |> _int_prim, 456 |> _int_prim),
          ),
          ("(123 / 456)", print(Divide, 123 |> _int_prim, 456 |> _int_prim)),
          (
            "Math.pow(123, 456)",
            print(Exponent, 123 |> _int_prim, 456 |> _int_prim),
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
          ("!true", print(Not, true |> _bool_prim)),
          ("+123", print(Positive, 123 |> _int_prim)),
          ("-123", print(Negative, 123 |> _int_prim)),
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
            ("Foo", [], []) |> of_tag |> print,
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
            ("Foo", [], []) |> of_tag |> of_node |> print,
          ),
          (
            "$knot.jsx.createTag(\"Foo\", { bar: fizz, foo: foo })",
            (
              "Foo",
              [
                ("foo", None) |> of_prop,
                ("bar", Some("fizz" |> _in_block |> of_id)) |> of_prop,
              ],
              [],
            )
            |> of_tag
            |> of_node
            |> print,
          ),
          (
            "$knot.jsx.createTag(\"Foo\", {}, $knot.jsx.createTag(\"Bar\", {}, \"fizz\"))",
            (
              "Foo",
              [],
              [("Bar", [], ["fizz" |> of_text]) |> of_tag |> of_node],
            )
            |> of_tag
            |> of_node
            |> print,
          ),
          ("\"Hello World!\"", "Hello World!" |> of_text |> print),
          ("123", 123 |> _int_prim |> of_inline_expr |> print),
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
          ("{ foo: foo }", [("foo", None) |> of_prop] |> print),
          (
            "{ foo: bar }",
            [("foo", Some("bar" |> _in_block |> of_id)) |> of_prop] |> print,
          ),
          (
            "{ className: \".foo\" }",
            [("foo", None) |> of_jsx_class] |> print,
          ),
          (
            "{ className: ((123 > 456) ? \".foo\" : \"\") + \".bar\" }",
            [
              ("bar", None) |> of_jsx_class,
              (
                "foo",
                Some((123 |> _int_prim, 456 |> _int_prim) |> of_gt_op),
              )
              |> of_jsx_class,
            ]
            |> print,
          ),
          ("{ id: \"foo\" }", ["foo" |> of_jsx_id] |> print),
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

        [("var foo = 123;\n", 123 |> _int_prim |> print("foo"))]
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
            123 |> _int_prim |> of_const |> print(Target.Common, "foo"),
          ),
          (
            "var foo = 123;
export { foo };
",
            123 |> _int_prim |> of_const |> print(Target.ES6, "foo"),
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
