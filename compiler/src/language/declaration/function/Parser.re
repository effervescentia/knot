open Knot.Kore;
open Parse.Kore;
open AST;

let function_ =
    ((ctx: ParseContext.t, tag_export: Raw.identifier_t => Module.export_t))
    : Framework.declaration_parser_t =>
  Matchers.keyword(Constants.Keyword.func)
  >>= Node.get_range
  % (
    start =>
      KIdentifier.Plugin.parse_id(ctx)
      >>= (
        id =>
          KExpression.Plugin.parse
          |> KLambda.Plugin.parse(ctx)
          >|= (
            ((args, res, range)) => {
              let scope = ctx |> Scope.of_parse_context(range);
              let args' =
                args
                |> KLambda.Plugin.analyze_argument_list(
                     scope,
                     KExpression.Plugin.analyze,
                   );

              args'
              |> List.iter(arg =>
                   scope
                   |> Scope.define(
                        Expression.(fst(arg).name) |> fst,
                        Node.get_type(arg),
                      )
                   |> Option.iter(
                        Scope.report_type_err(scope, Node.get_range(arg)),
                      )
                 );

              let res_scope =
                scope |> Scope.create_child(Node.get_range(res));
              let res' = res |> KExpression.Plugin.analyze(res_scope);

              let type_ =
                Type.Valid(
                  `Function((
                    args' |> List.map(Node.get_type),
                    Node.get_type(res'),
                  )),
                );
              let export_id = tag_export(id);

              ctx.symbols
              |> SymbolTable.declare_value(
                   ~main=Util.is_main(export_id),
                   fst(id),
                   type_,
                 );

              let func =
                Node.typed(
                  (args', res') |> AST.Result.of_func,
                  type_,
                  range,
                );

              Node.untyped((export_id, func), Range.join(start, range));
            }
          )
      )
      |> Matchers.terminated
  );
