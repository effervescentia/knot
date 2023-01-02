open Knot.Kore;
open Parse.Kore;
open AST;

let parse =
    ((ctx: ParseContext.t, export: Module.export_t))
    : Framework.declaration_parser_t =>
  Matchers.keyword(Constants.Keyword.const)
  >>= (
    kwd =>
      Matchers.assign(
        KIdentifier.Parser.parse_raw(ctx),
        KExpression.Plugin.parse(ctx),
      )
      >|= (
        ((id, raw_expr)) => {
          let scope =
            ctx |> Scope.of_parse_context(Node.get_range(raw_expr));
          let expr = raw_expr |> KExpression.Plugin.analyze(scope);
          let type_ = Node.get_type(expr);
          let const = expr |> Node.wrap(Result.of_const);
          let range = Node.join_ranges(kwd, raw_expr);

          // TODO: throw error if name already used in scope

          ctx.symbols
          |> SymbolTable.declare_value(
               ~main=Util.is_main(export),
               fst(id),
               type_,
             );

          Node.untyped((id, const), range);
        }
      )
      |> Matchers.terminated
  );
