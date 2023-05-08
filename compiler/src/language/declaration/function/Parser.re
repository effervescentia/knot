open Kore;
open Parse.Kore;
open AST;

let parse: Interface.Plugin.parse_t('ast) =
  (is_main, ctx) =>
    Matchers.keyword(Constants.Keyword.func)
    >>= Node.get_range
    % (
      start =>
        KIdentifier.Plugin.parse_raw(ctx)
        >>= (
          name =>
            Expression.parse
            |> Lambda.parse_lambda(ctx)
            >|= (
              ((parameters, body, lambda_range)) => {
                let scope = ctx |> Scope.of_parse_context(lambda_range);
                let parameters' =
                  parameters
                  |> Lambda.analyze_parameter_list(Expression.analyze, scope);

                parameters'
                |> List.iter(parameter =>
                     scope
                     |> Scope.define(
                          parameter |> fst |> Tuple.fst3 |> fst,
                          Node.get_type(parameter),
                        )
                     |> Option.iter(
                          Scope.report_type_err(
                            scope,
                            Node.get_range(parameter),
                          ),
                        )
                   );

                let body_scope =
                  scope |> Scope.create_child(Node.get_range(body));
                let (body', body_type) =
                  body |> Node.analyzer(Expression.analyze(body_scope));

                let type_ =
                  Type.Valid(
                    Function(
                      parameters' |> List.map(Node.get_type),
                      body_type,
                    ),
                  );

                ctx.symbols
                |> SymbolTable.declare_value(~main=is_main, fst(name), type_);

                let result =
                  Node.typed((parameters', body'), type_, lambda_range);
                let range = Range.join(start, lambda_range);

                Node.raw((name, result), range);
              }
            )
        )
        |> Matchers.terminated
    );
