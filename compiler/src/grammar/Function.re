open Kore;

module SemanticAnalyzer = Analyze.Semantic;

let parser = (ctx: ModuleContext.t, f): declaration_parser_t =>
  Keyword.func
  >>= N2.get_range
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
              |> List.iter(arg =>
                   scope
                   |> S.define(A.(fst(arg).name) |> fst, N2.get_type(arg))
                   |> Option.iter(
                        S.report_type_err(scope, N2.get_range(arg)),
                      )
                 );

              let res_scope = scope |> S.create_child(N2.get_range(raw_res));
              let res =
                raw_res |> SemanticAnalyzer.analyze_expression(res_scope);

              let type_ =
                T.Valid(
                  `Function((
                    args |> List.map(N2.get_type),
                    N2.get_type(res),
                  )),
                );
              let export_id = f(id);

              ctx
              |> ModuleContext.declare(
                   ~main=Util.is_main(export_id),
                   fst(id),
                   type_,
                 );

              let func = N2.typed((args, res) |> A.of_func, type_, range);

              N2.untyped((export_id, func), Range.join(start, range));
            }
          )
      )
      |> M.terminated
  );
