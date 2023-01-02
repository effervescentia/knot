open Kore;

let format:
  Fmt.t(AST.Result.raw_expression_t) => Fmt.t(AST.Result.raw_statement_t) =
  (pp_expression, ppf) => {
    let bind = format => format(pp_expression, ppf);

    Util.fold(
      ~variable=bind(KVariable.format),
      ~effect=bind(KEffect.format),
    );
  };
