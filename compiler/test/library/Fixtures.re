open Kore;

module A = AST.Result;
module AE = AST.Expression;
module U = Util.ResultUtil;
module ExportKind = AST.ModuleStatement.ExportKind;

/**
 `const nil_const = nil;`
 */
let nil_const = [
  (
    ExportKind.Named,
    "nil_const" |> U.as_untyped,
    U.nil_prim |> A.of_const |> U.as_nil,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const int_const = 123;`
 */
let int_const = [
  (
    ExportKind.Named,
    "int_const" |> U.as_untyped,
    123 |> U.int_prim |> A.of_const |> U.as_int,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const float_const = 123.000;`
 */
let float_const = [
  (
    ExportKind.Named,
    "float_const" |> U.as_untyped,
    (123.0, 3) |> U.float_prim |> A.of_const |> U.as_float,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const bool_const = true;`
 */
let bool_const = [
  (
    ExportKind.Named,
    "bool_const" |> U.as_untyped,
    true |> U.bool_prim |> A.of_const |> U.as_bool,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const string_const = "foo";`
 */
let string_const = [
  (
    AST.ModuleStatement.ExportKind.Named,
    "string_const" |> U.as_untyped,
    "foo" |> U.string_prim |> A.of_const |> U.as_string,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const identifier_const = foo;`
 */
let identifier_const = [
  (
    ExportKind.Named,
    "identifier_const" |> U.as_untyped,
    "foo" |> A.of_id |> U.as_int |> A.of_const |> U.as_int,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const jsx_const = <Foo />;`
 */
let jsx_const = [
  (
    ExportKind.Named,
    "jsx_const" |> U.as_untyped,
    ("Foo" |> U.as_view([], Valid(Nil)), [], [], [])
    |> U.ksx_tag
    |> U.as_element
    |> A.of_const
    |> U.as_element,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const group_const = (123);`
 */
let group_const = [
  (
    ExportKind.Named,
    "group_const" |> U.as_untyped,
    123 |> U.int_prim |> A.of_group |> U.as_int |> A.of_const |> U.as_int,
  )
  |> A.of_export
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
    ExportKind.Named,
    "closure_const" |> U.as_untyped,
    [
      (U.as_untyped("foo"), 123 |> U.int_prim) |> A.of_var |> U.as_nil,
      (U.as_untyped("bar"), U.nil_prim) |> A.of_var |> U.as_nil,
      false |> U.bool_prim |> A.of_effect |> U.as_bool,
    ]
    |> A.of_closure
    |> U.as_bool
    |> A.of_const
    |> U.as_bool,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const and_bool_const = true && false;`
 */
let and_bool_const = [
  (
    ExportKind.Named,
    "and_bool_const" |> U.as_untyped,
    (true |> U.bool_prim, false |> U.bool_prim)
    |> A.of_and_op
    |> U.as_bool
    |> A.of_const
    |> U.as_bool,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `const negative_int_const = -123;`
 */
let negative_int_const = [
  (
    ExportKind.Named,
    "negative_int_const" |> U.as_untyped,
    123 |> U.int_prim |> A.of_neg_op |> U.as_int |> A.of_const |> U.as_int,
  )
  |> A.of_export
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
    ExportKind.Named,
    "complex_jsx_const" |> U.as_untyped,
    (
      "Foo" |> U.as_view([], Valid(Nil)),
      [],
      [(U.as_untyped("buzz"), None) |> U.as_untyped],
      [
        ("Bar" |> U.as_view([], Valid(Nil)), [], [], [])
        |> A.of_element_tag
        |> A.of_node
        |> U.as_untyped,
        U.nil_prim |> A.of_inline_expr |> U.as_untyped,
        "fizzbuzz" |> A.of_text |> U.as_untyped,
        [] |> A.of_frag |> A.of_node |> U.as_untyped,
      ],
    )
    |> U.ksx_tag
    |> U.as_element
    |> A.of_const
    |> U.as_element,
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `func inline_function(foo, bar = 3) -> foo + bar;`
 */
let inline_function = [
  (
    ExportKind.Named,
    "inline_function" |> U.as_untyped,
    (
      [
        (U.as_untyped("foo"), None, None) |> U.as_int,
        (U.as_untyped("bar"), None, Some(3 |> U.int_prim)) |> U.as_int,
      ],
      ("foo" |> A.of_id |> U.as_int, "bar" |> A.of_id |> U.as_int)
      |> A.of_add_op
      |> U.as_int,
    )
    |> A.of_func
    |> U.as_function([Valid(Integer), Valid(Integer)], Valid(Integer)),
  )
  |> A.of_export
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
    ExportKind.Named,
    "multiline_function" |> U.as_untyped,
    (
      [],
      [
        ("zip" |> U.as_untyped, 3 |> U.int_prim) |> A.of_var |> U.as_nil,
        ("zap" |> U.as_untyped, 4 |> U.int_prim) |> A.of_var |> U.as_nil,
        ("zip" |> A.of_id |> U.as_int, "zap" |> A.of_id |> U.as_int)
        |> A.of_mult_op
        |> U.as_int
        |> A.of_effect
        |> U.as_int,
      ]
      |> A.of_closure
      |> U.as_int,
    )
    |> A.of_func
    |> U.as_function([Valid(Integer), Valid(Integer)], Valid(Integer)),
  )
  |> A.of_export
  |> U.as_untyped,
];

/**
 `import Foo from "main_import"`
 */
let main_import = [
  (
    Reference.Namespace.External("main_import"),
    "Foo" |> U.as_untyped |> Option.some,
    [],
  )
  |> A.of_import
  |> U.as_untyped,
];

/**
 `import { foo } from "named_import";`
 */
let named_import = [
  (
    Reference.Namespace.External("named_import"),
    None,
    [(U.as_untyped("foo"), None) |> U.as_untyped],
  )
  |> A.of_import
  |> U.as_untyped,
];
