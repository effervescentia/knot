open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.func
  >>= NR.get_range
  % (
    start =>
      Identifier.parser(ctx)
      >>= (
        id =>
          Lambda.parser(ctx)
          >|= (
            ((raw_args, raw_res, range)) => {
              let scope = ctx |> Util.create_scope(range);
              let args =
                raw_args |> SemanticAnalyzer.analyze_argument_list(scope);

              args
              |> List.iter(((arg, arg_type, arg_range)) =>
                   scope
                   |> S.define(A.(arg.name) |> NR.get_value, arg_type)
                   |> Option.iter(S.report_type_err(scope, arg_range))
                 );

              let res_scope = scope |> S.create_child(N.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_expression(res_scope);

              let type_ =
                T.Valid(
                  `Function((
                    args |> List.map(N.get_type),
                    N.get_type(res),
                  )),
                );
              let export_id = f(id);

              ctx
              |> ModuleContext.declare(
                   ~main=Util.is_main(export_id),
                   NR.get_value(id),
                   type_,
                 );

              let func = N.create((args, res) |> A.of_func, type_, range);

              NR.create((export_id, func), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );
