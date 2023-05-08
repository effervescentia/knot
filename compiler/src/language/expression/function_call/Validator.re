open Knot.Kore;
open AST;

let validate: ((Type.t, list(Type.t))) => option(Type.error_t) =
  fun
  /* assume this have been reported already and ignore */
  | (Invalid(_), _) => None

  | (Valid(Function(parameters, _)) as func_type, arguments) =>
    if (List.length(parameters) != List.length(arguments)) {
      Some(InvalidFunctionCall(func_type, arguments));
    } else {
      List.combine(parameters, arguments)
      |> List.fold_left(
           (err, pair) =>
             Option.(
               switch (err, pair) {
               | (Some(_), _) => err

               | (_, (Type.Valid(_) as expected, Type.Valid(_) as actual))
                   when expected == actual =>
                 None

               /* ignore it if the actual arg type is invalid */
               | (_, (Type.Valid(_), Type.Invalid(_))) => None

               | _ => Some(Type.InvalidFunctionCall(func_type, arguments))
               }
             ),
           None,
         );
    }

  | (expr_type, args) => Some(InvalidFunctionCall(expr_type, args));
