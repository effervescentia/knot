open Knot.Kore;
open AST;

let validate: ((Type.t, list(Type.t))) => option(Type.error_t) =
  fun
  /* assume this have been reported already and ignore */
  | (Invalid(_), _) => None

  | (Valid(`Function(args, _)) as func_type, actual_args) =>
    if (List.length(args) != List.length(actual_args)) {
      Some(InvalidFunctionCall(func_type, actual_args));
    } else {
      List.combine(args, actual_args)
      |> List.fold_left(
           (err, args) =>
             Option.(
               switch (err, args) {
               | (Some(_), _) => err

               | (_, (Type.Valid(_), Type.Valid(_)))
                   when fst(args) == snd(args) =>
                 None

               /* ignore it if the actual arg type is invalid */
               | (_, (Type.Valid(_), Type.Invalid(_))) => None

               | _ => Some(Type.InvalidFunctionCall(func_type, actual_args))
               }
             ),
           None,
         );
    }

  | (expr_type, args) => Some(InvalidFunctionCall(expr_type, args));