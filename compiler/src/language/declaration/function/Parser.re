open Knot.Kore;
open Parse.Kore;
open AST;

let parse = ((ctx: ParseContext.t, export: Module.export_kind_t)) =>
  Matchers.keyword(Constants.Keyword.func)
  >>= Node.get_range
  % (
    start =>
      KIdentifier.Parser.parse_raw(ctx)
      >>= (
        name =>
          KExpression.Plugin.parse
          |> KLambda.Parser.parse_lambda(ctx)
          >|= (
            ((parameters, body, lambda_range)) => {
              let scope = ctx |> Scope.of_parse_context(lambda_range);
              let parameters' =
                parameters
                |> KLambda.Analyzer.analyze_argument_list(
                     scope,
                     KExpression.Plugin.analyze,
                   );

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
              let body' = body |> KExpression.Plugin.analyze(body_scope);

              let type_ =
                Type.Valid(
                  Function(
                    parameters' |> List.map(Node.get_type),
                    Node.get_type(body'),
                  ),
                );

              ctx.symbols
              |> SymbolTable.declare_value(
                   ~main=Util.is_main(export),
                   fst(name),
                   type_,
                 );

              let result =
                Node.typed((parameters', body'), type_, lambda_range);
              let range = Range.join(start, lambda_range);

              Node.raw((name, result), range);
            }
          )
      )
      |> Matchers.terminated
  );
