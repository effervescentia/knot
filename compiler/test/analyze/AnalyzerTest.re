open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let _create_scope = name =>
  Scope.create(Reference.Namespace.of_string(name), Range.zero, ignore);
let _create_id = Identifier.of_string % CommonUtil.as_raw_node;

let __empty_scope = _create_scope("foo");
let __id = _create_id("foo");

let suite =
  "Analyze.Analyzer"
  >::: [
    "res_prim()"
    >: (
      () =>
        [
          ((Type.Valid(`Nil), AST.Nil), AST.Raw.Nil),
          (
            (Type.Valid(`Boolean), AST.Boolean(true)),
            AST.Raw.Boolean(true),
          ),
          (
            (Type.Valid(`Integer), AST.Number(Integer(123L))),
            AST.Raw.Number(Integer(123L)),
          ),
          (
            (Type.Valid(`Float), AST.Number(Float(123.0, 3))),
            AST.Raw.Number(Float(123.0, 3)),
          ),
          (
            (Type.Valid(`String), AST.String("foo")),
            AST.Raw.String("foo"),
          ),
        ]
        |> List.map(
             Tuple.map_each2(
               Tuple.join2(ResultUtil.as_node),
               RawUtil.as_raw_node % Analyzer.res_prim,
             ),
           )
        |> Assert.(test_many(primitive))
    ),
    "res_expr() - primitive"
    >: (
      () =>
        [
          (
            ResultUtil.int_prim(123),
            RawUtil.(__empty_scope, int_prim(123)),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_expr)))
        |> Assert.(test_many(expression))
    ),
    "res_expr() - jsx"
    >: (
      () =>
        [
          (
            ResultUtil.((__id, [], []) |> jsx_tag |> as_element),
            RawUtil.(
              __empty_scope,
              (__id, [], []) |> jsx_tag |> as_raw_node,
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_expr)))
        |> Assert.(test_many(expression))
    ),
    "res_expr() - group"
    >: (
      () =>
        [
          (
            ResultUtil.(
              AST.((123.0, 3) |> float_prim |> of_group |> as_float)
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.((123.0, 3) |> float_prim |> of_group |> as_raw_node),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_expr)))
        |> Assert.(test_many(expression))
    ),
    "res_expr() - closure"
    >: (
      () =>
        [
          (
            ResultUtil.(AST.([] |> of_closure |> as_nil)),
            RawUtil.(
              __empty_scope,
              AST.Raw.([] |> of_closure |> as_raw_node),
            ),
          ),
          (
            ResultUtil.(
              AST.(
                [
                  123 |> int_prim |> of_expr |> as_int,
                  nil_prim |> of_expr |> as_nil,
                  true |> bool_prim |> of_expr |> as_bool,
                ]
                |> of_closure
                |> as_bool
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                [
                  123 |> int_prim |> of_expr |> as_raw_node,
                  nil_prim |> of_expr |> as_raw_node,
                  true |> bool_prim |> of_expr |> as_raw_node,
                ]
                |> of_closure
                |> as_raw_node
              ),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_expr)))
        |> Assert.(test_many(expression))
    ),
    "res_jsx() - tag"
    >: (
      () =>
        [
          (
            ResultUtil.(AST.((__id, [], []) |> of_tag |> as_element)),
            RawUtil.(
              __empty_scope,
              AST.Raw.((__id, [], []) |> of_tag |> as_raw_node),
            ),
          ),
          (
            ResultUtil.(
              AST.(
                (
                  __id,
                  [],
                  [(_create_id("bar"), [], []) |> jsx_node |> as_element],
                )
                |> of_tag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                (
                  __id,
                  [],
                  [(_create_id("bar"), [], []) |> jsx_node |> as_raw_node],
                )
                |> of_tag
                |> as_raw_node
              ),
            ),
          ),
          (
            ResultUtil.(
              AST.(
                (
                  __id,
                  [],
                  [[] |> of_frag |> as_element |> of_node |> as_element],
                )
                |> of_tag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                (
                  __id,
                  [],
                  [[] |> of_frag |> as_raw_node |> of_node |> as_raw_node],
                )
                |> of_tag
                |> as_raw_node
              ),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_jsx)))
        |> Assert.(test_many(jsx))
    ),
    "res_jsx() - fragment"
    >: (
      () =>
        [
          (
            ResultUtil.(AST.([] |> of_frag |> as_element)),
            RawUtil.(__empty_scope, AST.Raw.([] |> of_frag |> as_raw_node)),
          ),
          (
            ResultUtil.(
              AST.(
                [(_create_id("bar"), [], []) |> jsx_node |> as_element]
                |> of_frag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                [(_create_id("bar"), [], []) |> jsx_node |> as_raw_node]
                |> of_frag
                |> as_raw_node
              ),
            ),
          ),
          (
            ResultUtil.(
              AST.(
                [[] |> of_frag |> as_element |> of_node |> as_element]
                |> of_frag
                |> as_element
              )
            ),
            RawUtil.(
              __empty_scope,
              AST.Raw.(
                [[] |> of_frag |> as_raw_node |> of_node |> as_raw_node]
                |> of_frag
                |> as_raw_node
              ),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_jsx)))
        |> Assert.(test_many(jsx))
    ),
    "res_stmt() - variable"
    >: (
      () =>
        [
          (
            ResultUtil.(AST.((__id, int_prim(123)) |> of_var |> as_nil)),
            RawUtil.(
              __empty_scope,
              AST.Raw.((__id, int_prim(123)) |> of_var |> as_raw_node),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_stmt)))
        |> Assert.(test_many(statement))
    ),
    "res_stmt() - expression"
    >: (
      () =>
        [
          (
            ResultUtil.(AST.(123 |> int_prim |> of_expr |> as_int)),
            RawUtil.(
              __empty_scope,
              AST.Raw.(123 |> int_prim |> of_expr |> as_raw_node),
            ),
          ),
        ]
        |> List.map(Tuple.map_snd2(Tuple.join2(Analyzer.res_stmt)))
        |> Assert.(test_many(statement))
    ),
  ];
