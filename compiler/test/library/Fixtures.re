open Kore;

module A = AST.Result;
module AE = AST.Expression;
module U = Util.ResultUtil;

/**
 `const nil_const = nil;`
 */
let nil_const = [
  (
    "nil_const" |> U.as_untyped |> A.of_named_export,
    U.nil_prim |> A.of_const |> U.as_nil,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const int_const = 123;`
 */
let int_const = [
  (
    "int_const" |> U.as_untyped |> A.of_named_export,
    123 |> U.int_prim |> A.of_const |> U.as_int,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const float_const = 123.000;`
 */
let float_const = [
  (
    "float_const" |> U.as_untyped |> A.of_named_export,
    (123.0, 3) |> U.float_prim |> A.of_const |> U.as_float,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const bool_const = true;`
 */
let bool_const = [
  (
    "bool_const" |> U.as_untyped |> A.of_named_export,
    true |> U.bool_prim |> A.of_const |> U.as_bool,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const string_const = "foo";`
 */
let string_const = [
  (
    "string_const" |> U.as_untyped |> A.of_named_export,
    "foo" |> U.string_prim |> A.of_const |> U.as_string,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const identifier_const = foo;`
 */
let identifier_const = [
  (
    "identifier_const" |> U.as_untyped |> A.of_named_export,
    "foo" |> A.of_id |> U.as_int |> A.of_const |> U.as_int,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const jsx_const = <Foo />;`
 */
let jsx_const = [
  (
    "jsx_const" |> U.as_untyped |> A.of_named_export,
    (U.as_untyped("Foo"), [], [])
    |> U.jsx_tag
    |> U.as_element
    |> A.of_const
    |> U.as_element,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const group_const = (123);`
 */
let group_const = [
  (
    "group_const" |> U.as_untyped |> A.of_named_export,
    123 |> U.int_prim |> A.of_group |> U.as_int |> A.of_const |> U.as_int,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const closure_const = {
   let foo = 123;
   let bar = nil;
   false;
 };`
 */
let closure_const = [
  (
    "closure_const" |> U.as_untyped |> A.of_named_export,
    [
      (U.as_untyped("foo"), 123 |> U.int_prim) |> A.of_var |> U.as_nil,
      (U.as_untyped("bar"), U.nil_prim) |> A.of_var |> U.as_nil,
      false |> U.bool_prim |> A.of_expr |> U.as_bool,
    ]
    |> A.of_closure
    |> U.as_bool
    |> A.of_const
    |> U.as_bool,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const and_bool_const = true && false;`
 */
let and_bool_const = [
  (
    "and_bool_const" |> U.as_untyped |> A.of_named_export,
    (true |> U.bool_prim, false |> U.bool_prim)
    |> A.of_and_op
    |> U.as_bool
    |> A.of_const
    |> U.as_bool,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const negative_int_const = -123;`
 */
let negative_int_const = [
  (
    "negative_int_const" |> U.as_untyped |> A.of_named_export,
    123 |> U.int_prim |> A.of_neg_op |> U.as_int |> A.of_const |> U.as_int,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `const complex_jsx_const = <Foo #bar .fizz buzz>
    <Bar />
    {nil}
    fizzbuzz
    <></>
 </Foo>;`
 */
let complex_jsx_const = [
  (
    "complex_jsx_const" |> U.as_untyped |> A.of_named_export,
    (
      U.as_untyped("Foo"),
      [(U.as_untyped("buzz"), None) |> A.of_prop |> U.as_untyped],
      [
        (U.as_untyped("Bar"), [], []) |> A.of_tag |> A.of_node |> U.as_untyped,
        U.nil_prim |> A.of_inline_expr |> U.as_untyped,
        "fizzbuzz" |> A.of_text |> U.as_untyped,
        [] |> A.of_frag |> A.of_node |> U.as_untyped,
      ],
    )
    |> U.jsx_tag
    |> U.as_element
    |> A.of_const
    |> U.as_element,
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `func inline_function(foo, bar = 3) -> foo + bar;`
 */
let inline_function = [
  (
    "inline_function" |> U.as_untyped |> A.of_named_export,
    (
      [
        AE.{name: U.as_untyped("foo"), default: None, type_: None} |> U.as_int,
        AE.{
          name: U.as_untyped("bar"),
          default: Some(3 |> U.int_prim),
          type_: None,
        }
        |> U.as_int,
      ],
      ("foo" |> A.of_id |> U.as_int, "bar" |> A.of_id |> U.as_int)
      |> A.of_add_op
      |> U.as_int,
    )
    |> A.of_func
    |> U.as_function([Valid(`Integer), Valid(`Integer)], Valid(`Integer)),
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `func multiline_function -> {
   let zip = 3;
   let zap = 4;
   zip * zap;
 }`
 */
let multiline_function = [
  (
    "multiline_function" |> U.as_untyped |> A.of_named_export,
    (
      [],
      [
        ("zip" |> U.as_untyped, 3 |> U.int_prim) |> A.of_var |> U.as_nil,
        ("zap" |> U.as_untyped, 4 |> U.int_prim) |> A.of_var |> U.as_nil,
        ("zip" |> A.of_id |> U.as_int, "zap" |> A.of_id |> U.as_int)
        |> A.of_mult_op
        |> U.as_int
        |> A.of_expr
        |> U.as_int,
      ]
      |> A.of_closure
      |> U.as_int,
    )
    |> A.of_func
    |> U.as_function([Valid(`Integer), Valid(`Integer)], Valid(`Integer)),
  )
  |> A.of_decl
  |> U.as_untyped,
];

/**
 `import Foo from "main_import"`
 */
let main_import = [
  (
    "main_import" |> A.of_external,
    ["Foo" |> U.as_untyped |> A.of_main_import |> U.as_untyped],
  )
  |> A.of_import
  |> U.as_untyped,
];

/**
 `import { foo } from "named_import";`
 */
let named_import = [
  (
    "named_import" |> A.of_external,
    [(U.as_untyped("foo"), None) |> A.of_named_import |> U.as_untyped],
  )
  |> A.of_import
  |> U.as_untyped,
];
