open Kore;

module URaw = Util.RawUtil;
module URes = Util.ResultUtil;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context: AST.ParseContext.t(Language.Interface.program_t(AST.Type.t)) =
  AST.ParseContext.create(~report=ignore, __namespace);
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
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __scope,
             )
          |> fst,
        )
    ),
    "extract type expression type"
    >: (
      () =>
        Assert.argument(
          (
            URes.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            None,
          )
          |> URes.as_bool,
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            None,
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __scope,
             )
          |> fst,
        )
    ),
    "resolve on matched valid types"
    >: (
      () =>
        Assert.argument(
          (
            URes.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            Some(URes.bool_prim(true)),
          )
          |> URes.as_bool,
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            Some(URaw.bool_prim(true)),
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __scope,
             )
          |> fst,
        )
    ),
    "resolve on invalid default expression type"
    >: (
      () =>
        Assert.argument(
          (
            URes.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            Some(
              __id
              |> Expression.of_identifier
              |> URes.as_invalid(NotInferrable),
            ),
          )
          |> URes.as_bool,
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            Some(__id |> Expression.of_identifier |> URaw.as_node),
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __scope,
             )
          |> fst,
        )
    ),
    "report TypeMismatch on unmatched types"
    >: (
      () =>
        Assert.throws_compile_errors(
          [
            ParseError(
              TypeError(TypeMismatch(Valid(Boolean), Valid(String))),
              __namespace,
              Range.zero,
            ),
          ],
          () =>
          (
            URaw.as_untyped(__id),
            Some(URaw.as_untyped(TypeExpression.Boolean)),
            Some(URaw.string_prim("bar")),
          )
          |> URaw.as_node
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __throw_scope,
             )
          |> fst
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
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __throw_scope,
             )
          |> fst
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
          |> KLambda.Analyzer.analyze_parameter(
               KExpression.Plugin.analyze,
               __scope,
             )
          |> fst,
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
              Some(URaw.as_untyped(TypeExpression.Boolean)),
              None,
            )
            |> URaw.as_node,
            (URaw.as_untyped("buzz"), None, Some(URaw.bool_prim(true)))
            |> URaw.as_node,
            (
              URaw.as_untyped("bar"),
              Some(URaw.as_untyped(TypeExpression.Boolean)),
              None,
            )
            |> URaw.as_node,
          ]
          |> KLambda.Analyzer.analyze_parameter_list(
               KExpression.Plugin.analyze,
               __throw_scope,
             )
        )
    ),
  ];
