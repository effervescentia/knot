open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let __empty_scope = Util.create_scope("foo");
let __id = Util.create_id("foo");

let _assert_statement = (expected, actual) =>
  Assert.analyzed_statement(
    expected,
    actual |> Tuple.join2(Analyzer.analyze_statement),
  );

let suite =
  "Analyze.Analyzer | Statement"
  >::: [
    "variable declaration"
    >: (
      () =>
        _assert_statement(
          AnalyzedUtil.(
            AST.Analyzed.((__id, int_prim(123)) |> of_var |> as_nil)
          ),
          RawUtil.(
            __empty_scope,
            AST.Raw.((__id, int_prim(123)) |> of_var |> as_raw_node),
          ),
        )
    ),
    "expression"
    >: (
      () =>
        _assert_statement(
          AnalyzedUtil.(AST.Analyzed.(123 |> int_prim |> of_expr |> as_int)),
          RawUtil.(
            __empty_scope,
            AST.Raw.(123 |> int_prim |> of_expr |> as_raw_node),
          ),
        )
    ),
  ];
