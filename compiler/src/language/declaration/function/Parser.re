open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;
module SemanticAnalyzer = Analyze.Semantic;
module Util = Grammar.Util;

let function_ =
    (
      ctx: ParseContext.t,
      tag_export: AST.Raw.identifier_t => AST.export_t,
      parse_expression: Grammar.Kore.contextual_expression_parser_t,
    )
    : Grammar.Kore.declaration_parser_t =>
  Keyword.func
  >>= Node.get_range
  % (
    start =>
      KIdentifier.Plugin.parse(ctx)
      >>= (
        id =>
          parse_expression
          |> KLambda.Plugin.parse(ctx)
          >|= (
            ((raw_args, raw_res, range)) => {
              let scope = ctx |> Scope.of_parse_context(range);
              let args =
                raw_args |> SemanticAnalyzer.analyze_argument_list(scope);

              args
              |> List.iter(arg =>
                   scope
                   |> Scope.define(
                        AST.(fst(arg).name) |> fst,
                        Node.get_type(arg),
                      )
                   |> Option.iter(
                        Scope.report_type_err(scope, Node.get_range(arg)),
                      )
                 );

              let res_scope =
                scope |> Scope.create_child(Node.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_expression(res_scope);

              let type_ =
                Type.Valid(
                  `Function((
                    args |> List.map(Node.get_type),
                    Node.get_type(res),
                  )),
                );
              let export_id = tag_export(id);

              ctx.symbols
              |> SymbolTable.declare_value(
                   ~main=Util.is_main(export_id),
                   fst(id),
                   type_,
                 );

              let func =
                Node.typed((args, res) |> AST.of_func, type_, range);

              Node.untyped((export_id, func), Range.join(start, range));
            }
          )
      )
      |> Matchers.terminated
  );
