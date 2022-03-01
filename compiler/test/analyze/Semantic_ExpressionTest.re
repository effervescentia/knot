open Kore;

module Identifier = Reference.Identifier;
module SemanticAnalyzer = Analyze.Semantic;
module URaw = Util.RawUtil;
module URes = Util.ResultUtilV2;

let __id = Identifier.of_string("foo");
let __namespace = Reference.Namespace.of_string("foo");
let __scope = S.create(__namespace, ignore, Range.zero);
let __throw_scope = S.create(__namespace, throw, Range.zero);

let suite =
  "Analyze.Semantic | Expression"
  >::: [
    "adopt primitive type"
    >: (
      () =>
        Assert.expression(
          URes.bool_prim(true),
          true
          |> URaw.bool_prim
          |> SemanticAnalyzer.analyze_expression(__scope),
        )
    ),
    "extract group inner expression type"
    >: (
      () =>
        Assert.expression(
          "foo" |> URes.string_prim |> A.of_group |> URes.as_string,
          "foo"
          |> URaw.string_prim
          |> AR.of_group
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(__scope),
        )
    ),
    "extract last closure statement type"
    >: (
      () =>
        Assert.expression(
          [123 |> URes.int_prim |> A.of_expr |> URes.as_int]
          |> A.of_closure
          |> URes.as_int,
          [123 |> URaw.int_prim |> AR.of_expr |> URaw.as_int]
          |> AR.of_closure
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(__scope),
        )
    ),
    "resolve nil if closure empty"
    >: (
      () =>
        Assert.expression(
          [] |> A.of_closure |> URes.as_nil,
          []
          |> AR.of_closure
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(__scope),
        )
    ),
    "always resolve JSX as element type"
    >: (
      () =>
        Assert.expression(
          (URes.as_raw_node(__id), [], [])
          |> A.of_tag
          |> A.of_jsx
          |> URes.as_element,
          (URaw.as_raw_node(__id), [], [])
          |> AR.of_tag
          |> AR.of_jsx
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(__scope),
        )
    ),
    "resolve NotInferrable type on unrecognized identifier"
    >: (
      () =>
        Assert.expression(
          __id |> A.of_id |> URes.as_invalid(NotInferrable),
          __id
          |> AR.of_id
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(__scope),
        )
    ),
    "report NotFound exception on unrecognized identifier"
    >: (
      () =>
        Assert.throws(
          CompileError([
            ParseError(
              TypeErrorV2(NotFound(__id)),
              __namespace,
              Range.zero,
            ),
          ]),
          "should throw NotFound",
          () =>
          __id
          |> AR.of_id
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(__throw_scope)
        )
    ),
    "resolve recognized identifier"
    >: (
      () => {
        let scope = {
          ...S.create(__namespace, throw, Range.zero),
          types:
            [(__id, T.Valid(`Boolean))] |> List.to_seq |> Hashtbl.of_seq,
        };

        Assert.expression(
          __id |> A.of_id |> URes.as_bool,
          __id
          |> AR.of_id
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_expression(scope),
        );
      }
    ),
  ];
