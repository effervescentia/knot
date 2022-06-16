open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.const
  >|= NR.get_range
  >>= (
    start =>
      Operator.assign(Identifier.parser(ctx), Expression.parser(ctx))
      >|= (
        ((id, (_, _, expr_range) as raw_expr)) => {
          let scope = ctx |> Util.create_scope(expr_range);
          let expr = raw_expr |> SemanticAnalyzer.analyze_expression(scope);
          let type_ = N.get_type(expr);
          let const = expr |> N.wrap(A.of_const);
          let range = Range.join(start, expr_range);
          let export_id = f(id);

          ctx
          |> ModuleContext.declare(
               ~main=Util.is_main(export_id),
               NR.get_value(id),
               type_,
             );

          NR.create((export_id, const), range);
        }
      )
      |> M.terminated
  );
