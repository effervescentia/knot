open Kore;
open AST;

let format: Fmt.t(Result.raw_expression_t) => Fmt.t(Result.raw_statement_t) =
  (pp_expression, ppf) => {
    let bind = format => format(pp_expression, ppf);

    Statement.fold(
      ~variable=bind(KVariable.format),
      ~effect=bind(KEffect.format),
    );
  };
