open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __empty_scope = Util.create_scope("foo");

let suite =
  "Analyze.Analyzer | Expression"
  >::: [
    "primitive"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.int_prim(123),
          RawUtil.(__empty_scope, int_prim(123)),
        )
    ),
    "jsx"
    >: (
      () => {
        let id = Util.create_id("foo");

        Assert.analyzed_expression(
          AnalyzedUtil.((id, [], []) |> jsx_tag |> as_element),
          RawUtil.(__empty_scope, (id, [], []) |> jsx_tag |> as_raw_node),
        );
      }
    ),
    "group"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.((123.0, 3) |> float_prim |> of_group |> as_float)
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.((123.0, 3) |> float_prim |> of_group |> as_raw_node),
          ),
        )
    ),
    "empty closure"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(AST.Analyzed.([] |> of_closure |> as_nil)),
          RawUtil.(__empty_scope, AST.Raw.([] |> of_closure |> as_raw_node)),
        )
    ),
    "closure with statements"
    >: (
      () =>
        Assert.analyzed_expression(
          AnalyzedUtil.(
            AST.Analyzed.(
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
        )
    ),
  ];
