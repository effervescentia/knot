open Knot.Kore;

let pp_boolean: Fmt.t(bool) =
  ppf =>
    (
      fun
      | true => Constants.Keyword.true_
      | false => Constants.Keyword.false_
    )
    % Fmt.string(ppf);
