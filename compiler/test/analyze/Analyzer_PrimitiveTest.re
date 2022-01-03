open Kore;
open Reference;
open Util;

module Analyzer = Analyze.Analyzer;

let _assert_primitive = (expected, actual) =>
  Assert.analyzed_primitive(
    expected |> Tuple.join2(AnalyzedUtil.as_node),
    actual |> RawUtil.as_raw_node |> Analyzer.analyze_primitive,
  );

let suite =
  "Analyze.Analyzer | Primitive"
  >::: [
    "nil"
    >: (
      () =>
        _assert_primitive(
          (Type.Raw.Strong(`Nil), AST.Analyzed.Nil),
          AST.Raw.Nil,
        )
    ),
    "boolean"
    >: (
      () =>
        _assert_primitive(
          (Type.Raw.Strong(`Boolean), AST.Analyzed.Boolean(true)),
          AST.Raw.Boolean(true),
        )
    ),
    "integer"
    >: (
      () =>
        _assert_primitive(
          (Type.Raw.Strong(`Integer), AST.Analyzed.Number(Integer(123L))),
          AST.Raw.Number(Integer(123L)),
        )
    ),
    "float"
    >: (
      () =>
        _assert_primitive(
          (Type.Raw.Strong(`Float), AST.Analyzed.Number(Float(123.0, 3))),
          AST.Raw.Number(Float(123.0, 3)),
        )
    ),
    "string"
    >: (
      () =>
        _assert_primitive(
          (Type.Raw.Strong(`String), AST.Analyzed.String("foo")),
          AST.Raw.String("foo"),
        )
    ),
  ];
