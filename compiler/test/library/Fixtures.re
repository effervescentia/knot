open Kore;
open AST;
open Reference;
open Util.ResultUtil;

/**
 `const nil_const = nil;`
 */
let nil_const = [
  (
    "nil_const" |> of_public |> as_raw_node |> of_named_export,
    nil_prim |> of_const |> as_nil,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const int_const = 123;`
 */
let int_const = [
  (
    "int_const" |> of_public |> as_raw_node |> of_named_export,
    123 |> int_prim |> of_const |> as_int,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const float_const = 123.000;`
 */
let float_const = [
  (
    "float_const" |> of_public |> as_raw_node |> of_named_export,
    (123.0, 3) |> float_prim |> of_const |> as_float,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const bool_const = true;`
 */
let bool_const = [
  (
    "bool_const" |> of_public |> as_raw_node |> of_named_export,
    true |> bool_prim |> of_const |> as_bool,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const string_const = "foo";`
 */
let string_const = [
  (
    "string_const" |> of_public |> as_raw_node |> of_named_export,
    "foo" |> string_prim |> of_const |> as_string,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const identifier_const = foo;`
 */
let identifier_const = [
  (
    "identifier_const" |> of_public |> as_raw_node |> of_named_export,
    "foo"
    |> Identifier.of_string
    |> as_int
    |> of_id
    |> as_int
    |> of_const
    |> as_int,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const jsx_const = <Foo />;`
 */
let jsx_const = [
  (
    "jsx_const" |> of_public |> as_raw_node |> of_named_export,
    ("Foo" |> Identifier.of_string |> as_raw_node, [], [])
    |> jsx_tag
    |> as_element
    |> of_const
    |> as_element,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const group_const = (123);`
 */
let group_const = [
  (
    "group_const" |> of_public |> as_raw_node |> of_named_export,
    123 |> int_prim |> of_group |> as_int |> of_const |> as_int,
  )
  |> of_decl
  |> as_raw_node,
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
    "closure_const" |> of_public |> as_raw_node |> of_named_export,
    [
      ("foo" |> Identifier.of_string |> as_raw_node, 123 |> int_prim)
      |> of_var
      |> as_nil,
      ("bar" |> Identifier.of_string |> as_raw_node, nil_prim)
      |> of_var
      |> as_nil,
      false |> bool_prim |> of_expr |> as_bool,
    ]
    |> of_closure
    |> as_bool
    |> of_const
    |> as_bool,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const and_bool_const = true && false;`
 */
let and_bool_const = [
  (
    "and_bool_const" |> of_public |> as_raw_node |> of_named_export,
    (true |> bool_prim, false |> bool_prim)
    |> of_and_op
    |> as_bool
    |> of_const
    |> as_bool,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `const negative_int_const = -123;`
 */
let negative_int_const = [
  (
    "negative_int_const" |> of_public |> as_raw_node |> of_named_export,
    123 |> int_prim |> of_neg_op |> as_int |> of_const |> as_int,
  )
  |> of_decl
  |> as_raw_node,
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
    "complex_jsx_const" |> of_public |> as_raw_node |> of_named_export,
    (
      "Foo" |> of_public |> as_raw_node,
      [
        "bar" |> of_public |> as_raw_node |> of_jsx_id |> as_string,
        ("fizz" |> of_public |> as_raw_node, None)
        |> of_jsx_class
        |> as_string,
        ("buzz" |> of_public |> as_raw_node, None)
        |> of_prop
        |> as_generic(0),
      ],
      [
        ("Bar" |> of_public |> as_raw_node, [], [])
        |> of_tag
        |> as_element
        |> of_node
        |> as_element,
        nil_prim |> of_inline_expr |> as_nil,
        "fizzbuzz" |> as_string |> of_text |> as_string,
        [] |> of_frag |> as_element |> of_node |> as_element,
      ],
    )
    |> jsx_tag
    |> as_element
    |> of_const
    |> as_element,
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `func inline_function(foo, bar = 3) -> foo + bar;`
 */
let inline_function = [
  (
    "inline_function" |> of_public |> as_raw_node |> of_named_export,
    (
      [
        {name: "foo" |> of_public |> as_raw_node, default: None, type_: None}
        |> as_int,
        {
          name: "bar" |> of_public |> as_raw_node,
          default: Some(3 |> int_prim),
          type_: None,
        }
        |> as_int,
      ],
      (
        "foo" |> of_public |> as_int |> of_id |> as_int,
        "bar" |> of_public |> as_int |> of_id |> as_int,
      )
      |> of_add_op
      |> as_int,
    )
    |> of_func
    |> as_function(
         [("foo", Valid(`Integer)), ("bar", Valid(`Integer))],
         Valid(`Integer),
       ),
  )
  |> of_decl
  |> as_raw_node,
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
    "multiline_function" |> of_public |> as_raw_node |> of_named_export,
    (
      [],
      [
        ("zip" |> of_public |> as_raw_node, 3 |> int_prim) |> of_var |> as_nil,
        ("zap" |> of_public |> as_raw_node, 4 |> int_prim) |> of_var |> as_nil,
        (
          "zip" |> of_public |> as_int |> of_id |> as_int,
          "zap" |> of_public |> as_int |> of_id |> as_int,
        )
        |> of_mult_op
        |> as_int
        |> of_expr
        |> as_int,
      ]
      |> of_closure
      |> as_int,
    )
    |> of_func
    |> as_function(
         [("foo", Valid(`Integer)), ("bar", Valid(`Integer))],
         Valid(`Integer),
       ),
  )
  |> of_decl
  |> as_raw_node,
];

/**
 `import Foo from "main_import"`
 */
let main_import = [
  (
    "main_import" |> of_external,
    ["Foo" |> of_public |> as_raw_node |> of_main_import |> as_raw_node],
  )
  |> of_import
  |> as_raw_node,
];

/**
 `import { foo } from "named_import";`
 */
let named_import = [
  (
    "named_import" |> of_external,
    [
      ("foo" |> of_public |> as_raw_node, None)
      |> of_named_import
      |> as_raw_node,
    ],
  )
  |> of_import
  |> as_raw_node,
];
