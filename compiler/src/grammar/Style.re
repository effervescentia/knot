open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let style_rule = (ctx: ParseContext.t, rule_scope: Scope.t) =>
  Identifier.parser(ctx)
  >>= (
    rule =>
      Symbol.colon
      >> Expression.parser(ctx)
      >|= (
        expr => {
          let found_type = rule_scope |> Scope.lookup(fst(rule));

          switch (found_type) {
          /* only allow single-argument void functions to be treated as rules */
          | Some(Valid(`Function([_], Valid(`Nil)))) => ()

          | _ =>
            ctx
            |> ParseContext.report(
                 TypeError(UnknownStyleRule(fst(rule))),
                 N.get_range(rule),
               )
          };

          N.untyped(
            (
              N.typed(
                fst(rule),
                found_type |?: T.Invalid(NotInferrable),
                N.get_range(rule),
              ),
              expr,
            ),
            N.join_ranges(rule, expr),
          );
        }
      )
  );

let parser: contextual_expression_parser_t =
  (ctx: ParseContext.t) => {
    let rule_scope = Scope.create(ctx, Range.zero);

    Scope.inject_plugin_types(~prefix="", StyleRule, rule_scope);

    Keyword.style
    >|= N.get_range
    >>= (
      start_range =>
        style_rule(ctx, rule_scope)
        |> M.comma_sep
        |> M.between(Symbol.open_closure, Symbol.close_closure)
        >|= (
          raw_rules => {
            let closure_range = N.get_range(raw_rules);
            let scope = ctx |> Scope.of_parse_context(closure_range);

            Scope.inject_plugin_types(StyleExpression, scope);

            N.typed(
              AR.of_style(raw_rules |> fst),
              `Style,
              Range.join(start_range, closure_range),
            );
          }
        )
    );
  };
