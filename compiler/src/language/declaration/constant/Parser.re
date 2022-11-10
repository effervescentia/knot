open Knot.Kore;
open Parse.Kore;
open AST;

let constant = (ctx: ParseContext.t, f): ParserTypes.declaration_parser_t =>
  Matchers.keyword(Constants.Keyword.const)
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
          let expr = raw_expr |> KExpression.Plugin.analyze(scope);
          let type_ = Node.get_type(expr);
          let const = expr |> Node.wrap(AST.Result.of_const);
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
