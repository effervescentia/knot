open Knot.Kore;
open Parse.Kore;
open AST;

let parse: Interface.Plugin.parse_t('ast) =
  (is_main, ctx) =>
    Matchers.keyword(Constants.Keyword.const)
    >>= (
      kwd =>
        Matchers.assign(
          KIdentifier.Parser.parse_raw(ctx),
          KExpression.Plugin.parse(ctx),
        )
        >|= (
          ((name, expression)) => {
            let scope =
              ctx |> Scope.of_parse_context(Node.get_range(expression));
            let (expression', expression_type) =
              expression |> Node.analyzer(KExpression.Plugin.analyze(scope));

            // TODO: throw error if name already used in scope

            ctx.symbols
            |> SymbolTable.declare_value(
                 ~main=is_main,
                 fst(name),
                 expression_type,
               );

            let result = expression' |> Node.wrap(Fun.id);
            let range = Node.join_ranges(kwd, expression);

            Node.raw((name, result), range);
          }
        )
        |> Matchers.terminated
    );
