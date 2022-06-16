open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.const
  >>= (
    kwd =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, raw_expr)) => {
          let scope = ctx |> Util.create_scope(N2.get_range(raw_expr));
          let expr = raw_expr |> SemanticAnalyzer.analyze_expression(scope);
          let type_ = N2.get_type(expr);
          let const = expr |> N2.wrap(A.of_const);
          let range = N2.join_ranges(kwd, raw_expr);
          let export_id = f(id);

          ctx
          |> ModuleContext.declare(
               ~main=Util.is_main(export_id),
               fst(id),
               type_,
             );

          N2.untyped((export_id, const), range);
        }
      )
      |> M.terminated
  );
