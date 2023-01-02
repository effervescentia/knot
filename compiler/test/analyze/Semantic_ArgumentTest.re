open Kore;

module A = AST.Result;
module AE = AST.Expression;
module AR = AST.Raw;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;
module T = AST.Type;
module TE = AST.TypeExpression;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = AST.ParseContext.create(~report=ignore, __namespace);
let __scope = AST.Scope.create(__context, Range.zero);
let __throw_scope =
  AST.Scope.create({...__context, report: AST.Error.throw}, Range.zero);

let suite =
  "Analyze.Semantic | Argument"
  >::: [
    "extract default value expression type"
    >: (
      () =>
        Assert.argument(
          (URes.as_untyped(__id), None, Some(URes.string_prim("bar")))
          |> URes.as_string,
          (URaw.as_untyped(__id), None, Some(URaw.string_prim("bar")))
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "extract type expression type"
    >: (
      () =>
        Assert.argument(
          (URes.as_untyped(__id), Some(URaw.as_untyped(TE.Boolean)), None)
          |> URes.as_bool,
          (URaw.as_untyped(__id), Some(URaw.as_untyped(TE.Boolean)), None)
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "resolve on matched valid types"
    >: (
      () =>
        Assert.argument(
          (
            URes.as_untyped(__id),
            Some(URaw.as_untyped(TE.Boolean)),
            Some(URes.bool_prim(true)),
          )
          |> URes.as_bool,
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TE.Boolean)),
            Some(URaw.bool_prim(true)),
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "resolve on invalid default expression type"
    >: (
      () =>
        Assert.argument(
          (
            URes.as_untyped(__id),
            Some(URaw.as_untyped(TE.Boolean)),
            Some(__id |> A.of_id |> URes.as_invalid(NotInferrable)),
          )
          |> URes.as_bool,
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TE.Boolean)),
            Some(__id |> AR.of_id |> URaw.as_node),
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "report TypeMismatch on unmatched types"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(TypeMismatch(T.Valid(`Boolean), T.Valid(`String))),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TE.Boolean)),
            Some(URaw.string_prim("bar")),
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __throw_scope,
               KExpression.Plugin.analyze,
             )
        )
    ),
    "report UntypedFunctionArgument when no type information available"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(UntypedFunctionArgument(__id)),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (URaw.as_untyped(__id), None, None)
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __throw_scope,
               KExpression.Plugin.analyze,
             )
        )
    ),
    "resolve NotInferrable when no type information available"
    >: (
      () =>
        Assert.argument(
          (URes.as_untyped(__id), None, None)
          |> URes.as_invalid(NotInferrable),
          (URaw.as_untyped(__id), None, None)
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_argument(
               __scope,
               KExpression.Plugin.analyze,
             ),
        )
    ),
    "report DefaultArgumentMissing on gaps in default arguments"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(DefaultArgumentMissing("bar")),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          [
            (
              URaw.as_untyped("fizz"),
              Some(URaw.as_untyped(TE.Boolean)),
              None,
            )
            |> URaw.as_node,
            (URaw.as_untyped("buzz"), None, Some(URaw.bool_prim(true)))
            |> URaw.as_node,
            (
              URaw.as_untyped("bar"),
              Some(URaw.as_untyped(TE.Boolean)),
              None,
            )
            |> URaw.as_node,
          ]
          |> KLambda.Analyzer.analyze_argument_list(
               __throw_scope,
               KExpression.Plugin.analyze,
             )
        )
    ),
  ];
