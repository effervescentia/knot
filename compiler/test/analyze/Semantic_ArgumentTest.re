open Kore;

module SemanticAnalyzer = Analyze.Semantic;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;
module TE = AST.TypeExpression;

let __id = "foo";
let __namespace = Reference.Namespace.of_string("foo");
let __context = ParseContext.create(~report=ignore, __namespace);
let __scope = S.create(__context, Range.zero);
let __throw_scope = S.create({...__context, report: throw}, Range.zero);

let suite =
  "Analyze.Semantic | Argument"
  >::: [
    "extract default value expression type"
    >: (
      () =>
        Assert.argument(
          A.{
            name: URes.as_untyped(__id),
            default: Some(URes.string_prim("bar")),
            type_: None,
          }
          |> URes.as_string,
          AR.{
            name: URaw.as_untyped(__id),
            default: Some(URaw.string_prim("bar")),
            type_: None,
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
        )
    ),
    "extract type expression type"
    >: (
      () =>
        Assert.argument(
          A.{
            name: URes.as_untyped(__id),
            default: None,
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URes.as_bool,
          AR.{
            name: URaw.as_untyped(__id),
            default: None,
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
        )
    ),
    "resolve on matched valid types"
    >: (
      () =>
        Assert.argument(
          A.{
            name: URes.as_untyped(__id),
            default: Some(URes.bool_prim(true)),
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URes.as_bool,
          AR.{
            name: URaw.as_untyped(__id),
            default: Some(URaw.bool_prim(true)),
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
        )
    ),
    "resolve on invalid default expression type"
    >: (
      () =>
        Assert.argument(
          A.{
            name: URes.as_untyped(__id),
            default: Some(__id |> A.of_id |> URes.as_invalid(NotInferrable)),
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URes.as_bool,
          AR.{
            name: URaw.as_untyped(__id),
            default: Some(__id |> AR.of_id |> URaw.as_unknown),
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
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
          AR.{
            name: URaw.as_untyped(__id),
            default: Some(URaw.string_prim("bar")),
            type_: Some(URaw.as_untyped(TE.Boolean)),
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__throw_scope)
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
          AR.{name: URaw.as_untyped(__id), default: None, type_: None}
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__throw_scope)
        )
    ),
    "resolve NotInferrable when no type information available"
    >: (
      () =>
        Assert.argument(
          A.{name: URes.as_untyped(__id), default: None, type_: None}
          |> URes.as_invalid(NotInferrable),
          AR.{name: URaw.as_untyped(__id), default: None, type_: None}
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
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
            AR.{
              name: URaw.as_untyped("fizz"),
              default: None,
              type_: Some(URaw.as_untyped(TE.Boolean)),
            }
            |> URaw.as_unknown,
            AR.{
              name: URaw.as_untyped("buzz"),
              default: Some(URaw.bool_prim(true)),
              type_: None,
            }
            |> URaw.as_unknown,
            AR.{
              name: URaw.as_untyped("bar"),
              default: None,
              type_: Some(URaw.as_untyped(TE.Boolean)),
            }
            |> URaw.as_unknown,
          ]
          |> SemanticAnalyzer.analyze_argument_list(__throw_scope)
        )
    ),
  ];
