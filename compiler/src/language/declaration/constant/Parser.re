open Knot.Kore;
open Parse.Onyx;

module Keyword = Grammar.Keyword;
module Matchers = Grammar.Matchers;
module SemanticAnalyzer = Analyze.Semantic;
module Util = Grammar.Util;

let constant = (ctx: ParseContext.t, f): Grammar.Kore.declaration_parser_t =>
  Keyword.const
  >>= (
    kwd =>
      Matchers.assign(
        KIdentifier.Plugin.parse(ctx),
        KExpression.Plugin.parse(ctx),
      )
      >|= (
        ((id, raw_expr)) => {
          let scope =
            ctx |> Scope.of_parse_context(Node.get_range(raw_expr));
          let expr = raw_expr |> SemanticAnalyzer.analyze_expression(scope);
          let type_ = Node.get_type(expr);
          let const = expr |> Node.wrap(AST.of_const);
          let range = Node.join_ranges(kwd, raw_expr);
          let export_id = f(id);

          ctx.symbols
          |> SymbolTable.declare_value(
               ~main=Util.is_main(export_id),
               fst(id),
               type_,
             );

          Node.untyped((export_id, const), range);
        }
      )
      |> Matchers.terminated
  );
