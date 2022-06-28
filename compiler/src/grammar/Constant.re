open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ParseContext.t, f): declaration_parser_t =>
  Keyword.const
  >>= (
    kwd =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, raw_expr)) => {
          let scope = ctx |> ParseContext.to_scope(N.get_range(raw_expr));
          let expr = raw_expr |> SemanticAnalyzer.analyze_expression(scope);
          let type_ = N.get_type(expr);
          let const = expr |> N.wrap(A.of_const);
          let range = N.join_ranges(kwd, raw_expr);
          let export_id = f(id);

          ctx.symbols
          |> SymbolTable.declare_value(
               ~main=Util.is_main(export_id),
               fst(id),
               type_,
             );

          N.untyped((export_id, const), range);
        }
      )
      |> M.terminated
  );
