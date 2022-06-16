open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let variant = (ctx: ModuleContext.t) =>
  Identifier.parser(ctx)
  >>= (
    id =>
      option(
        ([], NR.get_range(id)),
        Identifier.parser(ctx)
        |> many
        |> M.between(Symbol.open_group, Symbol.close_group),
      )
  );

/* let scope = ctx |> Util.create_scope(expr_range);
   let expr = raw_expr |> SemanticAnalyzer.analyze_expression(scope);
   let type_ = N.get_type(expr);
   let const = expr |> N.wrap(A.of_const);
   let range = Range.join(start, expr_range);
   let export_id = f(id); */

/* let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
   Keyword.enum
   >|= NR.get_range
   >>= (
     start =>
       optional(Symbol.vertical_bar)
       >> (Expression.parser(ctx) |> sep_by(Symbol.vertical_bar))
       |> Operator.assign(Identifier.parser(ctx))
       >|= (
         ((id, raw_variants)) => {

           let type_ = T.Valid(`String);
           let range = start;
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
   ); */
