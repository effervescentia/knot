open Kore;

module Identifier = Reference.Identifier;
module SemanticAnalyzer = Analyze.Semantic;
module URaw = Util.RawUtil;
module URes = Util.ResultUtil;
module TE = AST.TypeExpression;

let __id = Identifier.of_string("foo");
let __namespace = Reference.Namespace.of_string("foo");
let __scope = S.create(__namespace, ignore, Range.zero);
let __throw_scope = S.create(__namespace, throw, Range.zero);

let suite =
  "Analyze.Semantic | Argument"
  >::: [
    "extract default value expression type"
    >: (
      () =>
        Assert.argument(
          A.{
            name: URes.as_raw_node(__id),
            default: Some(URes.string_prim("bar")),
            type_: None,
          }
          |> URes.as_string,
          AR.{
            name: URaw.as_raw_node(__id),
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
            name: URes.as_raw_node(__id),
            default: None,
            type_: Some(URaw.as_raw_node(TE.Boolean)),
          }
          |> URes.as_bool,
          AR.{
            name: URaw.as_raw_node(__id),
            default: None,
            type_: Some(URaw.as_raw_node(TE.Boolean)),
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
            name: URes.as_raw_node(__id),
            default: Some(URes.bool_prim(true)),
            type_: Some(URaw.as_raw_node(TE.Boolean)),
          }
          |> URes.as_bool,
          AR.{
            name: URaw.as_raw_node(__id),
            default: Some(URaw.bool_prim(true)),
            type_: Some(URaw.as_raw_node(TE.Boolean)),
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
            name: URes.as_raw_node(__id),
            default: Some(__id |> A.of_id |> URes.as_invalid(NotInferrable)),
            type_: Some(URaw.as_raw_node(TE.Boolean)),
          }
          |> URes.as_bool,
          AR.{
            name: URaw.as_raw_node(__id),
            default: Some(__id |> AR.of_id |> URaw.as_unknown),
            type_: Some(URaw.as_raw_node(TE.Boolean)),
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
        )
    ),
    "report TypeMismatch on unmatched types"
    >: (
      () =>
        Assert.throws(
          CompileError([
            ParseError(
              TypeError(TypeMismatch(T.Valid(`Boolean), T.Valid(`String))),
              __namespace,
              Range.zero,
            ),
          ]),
          "should throw TypeMismatch error",
          () =>
          AR.{
            name: URaw.as_raw_node(__id),
            default: Some(URaw.string_prim("bar")),
            type_: Some(URaw.as_raw_node(TE.Boolean)),
          }
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__throw_scope)
        )
    ),
    "report UntypedFunctionArgument when no type information available"
    >: (
      () =>
        Assert.throws(
          CompileError([
            ParseError(
              TypeError(UntypedFunctionArgument(__id)),
              __namespace,
              Range.zero,
            ),
          ]),
          "should throw UntypedFunctionArgument error",
          () =>
          AR.{name: URaw.as_raw_node(__id), default: None, type_: None}
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__throw_scope)
        )
    ),
    "resolve NotInferrable when no type information available"
    >: (
      () =>
        Assert.argument(
          A.{name: URes.as_raw_node(__id), default: None, type_: None}
          |> URes.as_invalid(NotInferrable),
          AR.{name: URaw.as_raw_node(__id), default: None, type_: None}
          |> URaw.as_unknown
          |> SemanticAnalyzer.analyze_argument(__scope),
        )
    ),
    "report DefaultArgumentMissing on gaps in default arguments"
    >: (
      () =>
        Assert.throws(
          CompileError([
            ParseError(
              TypeError(
                DefaultArgumentMissing(Identifier.of_string("bar")),
              ),
              __namespace,
              Range.zero,
            ),
          ]),
          "should throw DefaultArgumentMissing error",
          () =>
          [
            AR.{
              name: "fizz" |> Identifier.of_string |> URaw.as_raw_node,
              default: None,
              type_: Some(URaw.as_raw_node(TE.Boolean)),
            }
            |> URaw.as_unknown,
            AR.{
              name: "buzz" |> Identifier.of_string |> URaw.as_raw_node,
              default: Some(URaw.bool_prim(true)),
              type_: None,
            }
            |> URaw.as_unknown,
            AR.{
              name: "bar" |> Identifier.of_string |> URaw.as_raw_node,
              default: None,
              type_: Some(URaw.as_raw_node(TE.Boolean)),
            }
            |> URaw.as_unknown,
          ]
          |> SemanticAnalyzer.analyze_argument_list(__throw_scope)
        )
    ),
  ];
